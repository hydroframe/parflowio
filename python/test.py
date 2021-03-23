import unittest
from pathlib import Path
from parflowio.pyParflowio import PFData
import numpy as np
import os
import hashlib
import time


def calculate_sha1_hash(filepath):
    # BUF_SIZE is totally arbitrary, change for your app!
    BUF_SIZE = 65536  # lets read stuff in 64kb chunks!

    sha1 = hashlib.sha1()
    with open(filepath, 'rb') as f:
        while True:
            data = f.read(BUF_SIZE)
            if not data:
                break
            sha1.update(data)
    return sha1.hexdigest()


def byte_compare_files(f1, f2):
    bufsize = 1
    cnt = 1
    with open(f1, 'rb') as fp1, open(f2, 'rb') as fp2:
        while True:
            b1 = fp1.read(bufsize)
            b2 = fp2.read(bufsize)
            if b1 != b2:
                return False, cnt
            if not b1:
                return True, cnt
            cnt = cnt + 1


class PFDataClassTests(unittest.TestCase):

    @classmethod
    def setUpClass(cls) -> None:
        os.chdir(os.path.join('..', 'tests', 'inputs'))
        cls.local_dir = Path(__file__).parent

    def test_empty_init(self):
        test = PFData()
        retval = test.loadHeader()
        self.assertNotEqual(0, retval, 'Empty class should error loading header')

    def test_bad_filename(self):
        test = PFData('bad_filename_not_exists.pfb')
        retval = test.loadHeader()
        self.assertNotEqual(0, retval, 'Bad filename should error loading header')

    def test_open_close(self):
        test = PFData()
        self.assertIsNone(test.close(), 'should be able to open and close an empty object')

    def test_good_filename(self):
        test = PFData(('press.init.pfb'))
        retval = test.loadHeader()
        self.assertEqual(0, retval, 'should load header of file that exists')
        self.assertEqual(41, test.getNX(), 'sample file should have 41 columns')
        self.assertEqual(41, test.getNY(), 'sample file should have 41 rows')
        self.assertEqual(50, test.getNZ(), 'sample file should have 50 z-layers')
        self.assertEqual(0, test.getX(), 'sample file starts at X=0')
        self.assertEqual(0, test.getY(), 'sample file starts at Y=0')
        self.assertEqual(0, test.getZ(), 'sample file starts at Z=0')
        self.assertEqual(16, test.getNumSubgrids(), 'sample file should have 16 subgrids')
        test.close()

    def test_validate_cell_values(self):
        test = PFData(('press.init.pfb'))
        retval = test.loadHeader()
        self.assertEqual(0, retval, 'should load header of file that exists')
        retval = test.loadData()
        self.assertEqual(0, retval, 'should load data from valid file')
        data = test.viewDataArray()
        self.assertIsNotNone(data, 'data from object should be available as python object')
        self.assertSequenceEqual((41, 41, 50), data.shape)
        self.assertAlmostEqual(98.003604098773, test(0, 0, 0), 12, 'valid data in cell (0,0,0)')
        self.assertAlmostEqual(97.36460429313328, test(40, 0, 0), 12, 'data in cell (40,0,0)')
        self.assertAlmostEqual(98.0043134691891, test(0, 1, 0), 12, 'data in cell (0, 1, 0)')
        self.assertAlmostEqual(98.00901307022781, test(1, 0, 0), 12, 'data in cell (1, 0, 0)')
        self.assertAlmostEqual(92.61370155558751, test(21, 1, 2), 12, 'data in cell (21, 1, 2)')
        self.assertAlmostEqual(7.98008728357588, test(0, 1, 45), 12, 'data in cell (0, 1, 45)')
        self.assertAlmostEqual(97.30205516102234, test(22, 1, 0), 12, 'valid data in cell (22,1,0)')
        self.assertEqual(test(0, 0, 0), data[0, 0, 0], 'data array and c array match values at (0,0,0)')
        self.assertEqual(test(40, 0, 0), data[40, 0, 0], 'data array and c array match values at (40,0,0)')
        self.assertEqual(test(0, 1, 0), data[0, 1, 0], 'data array and c array match values at (0,1,0)')
        self.assertEqual(test(1, 0, 0), data[1, 0, 0], 'data array and c array match values at (1,0,0)')
        self.assertEqual(test(21, 1, 2), data[21, 1, 2], 'data array and c array match values at (21,1,2)')
        self.assertEqual(test(0, 1, 45), data[0, 1, 45], 'data array and c array match values at (0,1,45)')
        self.assertEqual(test(22, 1, 0), data[22, 1, 0], 'data array and c array match values at (22,1,0)')
        test.close()

    def test_compare(self):
        test1 = PFData(('press.init.pfb'))
        test1.loadHeader()
        test1.loadData()

        test2 = PFData(('press.init.pfb'))
        test2.loadHeader()
        test2.loadData()

        self.assertEqual(PFData.differenceType_none, test1.compare(test2)[0], "test1 and test2 are the same")

        test1.setX(test1.getX()+1.0)
        self.assertEqual(PFData.differenceType_x, test1.compare(test2)[0], "The x values differ")
        test1.setX(test1.getX()-1.0)

        arr = test1.viewDataArray()
        arr[1][2][3] += 1.0
        ret, xyz = test1.compare(test2)
        self.assertEqual(PFData.differenceType_data, ret, "The data values differ")
        self.assertEqual((1, 2, 3), xyz, "The differing data's coordinates are correct")
        arr[1][2][3] -= 1.0

        test1.close()
        test2.close()

    def test_load_data_threaded(self):
        base = PFData(('press.init.pfb'))
        base.loadHeader()
        base.loadData()

        # 1 thread
        test1 = PFData(('press.init.pfb'))
        test1.loadHeader()
        test1.loadPQR()
        test1.loadDataThreaded(1)
        self.assertEqual(PFData.differenceType_none, base.compare(test1)[0], "base and test1 are the same")

        # 8 threads
        test8 = PFData(('press.init.pfb'))
        test8.loadHeader()
        test8.loadPQR()
        test8.loadDataThreaded(8)
        self.assertEqual(PFData.differenceType_none, base.compare(test8)[0], "base and test8 are the same")

        # 40 threads (more than the number of subgrids)
        test40 = PFData(('press.init.pfb'))
        test40.loadHeader()
        test40.loadPQR()
        test40.loadDataThreaded(40)
        self.assertEqual(PFData.differenceType_none, base.compare(test40)[0], "base and test40 are the same")

        base.close()
        test1.close()
        test8.close()
        test40.close()

    def test_load_data_threaded_perf(self):
        # loadData - Not using threads
        non_threaded_time = 0
        for i in range(10000):
            base = PFData(('LW.out.press.00000.pfb'))   # Little Washita pressure PFB with 2x2x1 grid size
            base.loadHeader()

            start = time.time_ns()
            base.loadData()
            non_threaded_time += time.time_ns() - start

        # loadDataThreaded - Using 4 threads
        num_threads = 4
        threaded_time = 0
        for i in range(10000):
            test = PFData(('LW.out.press.00000.pfb'))   # Little Washita pressure PFB with 2x2x1 grid size
            test.loadHeader()

            start = time.time_ns()
            test.loadPQR()                              # loadPQR() must be called before loadDataThreaded()
            test.loadDataThreaded(num_threads)
            threaded_time += time.time_ns() - start

        base.close()
        test.close()

        # loadDataThreaded() should have less total time spent than loadData()
        assert threaded_time < non_threaded_time, f'Using {num_threads} threads has degraded the performance of loadDataThreaded()'

        # Display performance increase in percent change
        pct_change = 100 * abs(threaded_time - non_threaded_time) / non_threaded_time
        print(f'{pct_change:.2f}% performance increase when using LoadDataThreaded() with {num_threads} threads')


    def test_read_write_data(self):
        test = PFData(('press.init.pfb'))
        retval = test.loadHeader()
        self.assertEqual(0, retval, 'should load header of file that exists')
        retval = test.loadPQR()
        self.assertEqual(0, retval, 'should load PQR of file that exists')
        retval = test.loadData()
        self.assertEqual(0, retval, 'should load data from valid file')
        retval = test.writeFile(('press.init.pfb.tmp'))
        self.assertEqual(0, retval, 'should write data from previously loaded file')

        data2 = PFData(('press.init.pfb.tmp'))
        data2.loadHeader()
        data2.loadData()
        data2.loadPQR()
        self.assertIsNone(np.testing.assert_array_equal(test.viewDataArray(),
                                                        data2.viewDataArray(),
                                                        'should read back same values we wrote'))
        in_file_hash = calculate_sha1_hash(('press.init.pfb'))
        out_file_hash = calculate_sha1_hash(('press.init.pfb.tmp'))

        # This assertion (that the files are identical) is failing in Python and in C++
        # because the original test input file was written by a tool that incorrectly set the value
        self.assertNotEqual(in_file_hash, out_file_hash, 'sha1 hash of input and output files should not match')

        same, byte_diff = byte_compare_files(('press.init.pfb'),
                                             ('press.init.pfb.tmp'))

        self.assertFalse(same, 'press.init.pfb should differ from version just written')
        self.assertEqual(92, byte_diff, 'first byte difference at byte 92')

        test.close()
        data2.close()
        os.remove(('press.init.pfb.tmp'))

    def test_manipulate_data(self):
        test = PFData(('press.init.pfb'))
        retval = test.loadHeader()
        self.assertEqual(0, retval, 'should load header of file that exists')
        retval = test.loadData()
        self.assertEqual(0, retval, 'should load data from valid file')
        test_data = test.viewDataArray()
        self.assertSequenceEqual((41, 41, 50), test_data.shape, 'test file array should have shape (41,41,50)')
        self.assertAlmostEqual(98.003604098773, test(0, 0, 0), 12, 'valid data in cell (0,0,0)')
        test_data[0, 0, 0] = 1
        test_data[40, 0, 0] = 1
        test_data[21, 1, 2] = 1
        self.assertEqual(1, test(0, 0, 0), 'data update affects underlying array')
        self.assertEqual(1, test(40, 0, 0), 'data update affects underlying array')
        self.assertEqual(1, test(21, 1, 2), 'data update affects underlying array')
        retval = test.writeFile(('press.init.pfb.tmp'))
        self.assertEqual(0, retval, 'able to write updated data to output file')
        test_read = PFData(('press.init.pfb.tmp'))
        test_read.loadHeader()
        test_read.loadData()
        self.assertEqual(1, test_read(0, 0, 0), 'updates to data written to file can be read back')
        self.assertEqual(1, test_read(40, 0, 0), 'updates to data written to file can be read back')
        self.assertEqual(1, test_read(21, 1, 2), 'updates to data written to file can be read back')
        test.close()
        test_read.close()
        os.remove(('press.init.pfb.tmp'))

    def test_dist_file(self):
        test = PFData(('press.init.pfb'))
        test.distFile(P=2, Q=2, R=1, outFile=('press.init.pfb.tmp'))

        out_file = PFData(('press.init.pfb.tmp'))
        dist_file = open(('press.init.pfb.tmp.dist'), 'r')
        dist_lines = dist_file.readlines()
        [self.assertEqual(int(line.rstrip('\n')), val) for line, val in zip(dist_lines, [0, 176500, 344536, 512572,
                                                                                        672608])]
        self.assertEqual(0, out_file.loadHeader(), 'should load distributed file header')
        self.assertEqual(0, out_file.loadData(), 'should load distributed data')
        self.assertIsNone(np.testing.assert_array_equal(test.viewDataArray(), out_file.viewDataArray()),
                          'should find matching data values in original and distributed files')
        test.close()
        out_file.close()
        dist_file.close()
        os.remove(('press.init.pfb.tmp'))
        os.remove(('press.init.pfb.tmp.dist'))

    def test_dist_nldas_file(self):
        test = PFData(('NLDAS.APCP.000001_to_000024.pfb'))
        test.distFile(P=2, Q=2, R=1, outFile=('NLDAS.APCP.000001_to_000024.pfb.tmp'))

        out_file = PFData(('NLDAS.APCP.000001_to_000024.pfb.tmp'))
        dist_file = open(('NLDAS.APCP.000001_to_000024.pfb.tmp.dist'), 'r')
        dist_lines = dist_file.readlines()
        [self.assertEqual(int(line.rstrip('\n')), val) for line, val in zip(dist_lines, [0, 84772, 165448, 246124,
                                                                                         322960])]
        self.assertEqual(0, out_file.loadHeader(), 'should load distributed file header')
        self.assertEqual(0, out_file.loadData(), 'should load distributed data')
        self.assertIsNone(np.testing.assert_array_equal(test.viewDataArray(), out_file.viewDataArray()),
                          'should find matching data values in original and distributed files')
        test.close()
        out_file.close()
        dist_file.close()
        os.remove(('NLDAS.APCP.000001_to_000024.pfb.tmp'))
        os.remove(('NLDAS.APCP.000001_to_000024.pfb.tmp.dist'))

    def test_assign_data(self):
        test = PFData()
        data = np.random.random_sample((50, 49, 31))
        test.setDataArray(data)
        self.assertEqual(50, test.getNX())
        self.assertEqual(49, test.getNY())
        self.assertEqual(31, test.getNZ())
        test.setDX(1)
        test.setDY(1)
        test.setDZ(1)
        test.setX(0)
        test.setY(0)
        test.setZ(0)
        test.setP(1)
        test.setQ(1)
        test.setR(1)
        test.writeFile(('test_write_raw.pfb'))
        test_read = PFData(('test_write_raw.pfb'))
        test_read.loadHeader()
        test_read.loadData()
        self.assertEqual(0, test_read.getX())
        self.assertEqual(0, test_read.getY())
        self.assertEqual(0, test_read.getZ())
        self.assertEqual(50, test_read.getNX())
        self.assertEqual(49, test_read.getNY())
        self.assertEqual(31, test_read.getNZ())
        self.assertEqual(1, test_read.getP())
        self.assertEqual(1, test_read.getQ())
        self.assertEqual(1, test_read.getR())
        test_data = test_read.viewDataArray()
        self.assertIsNone(np.testing.assert_array_equal(data, test_data), 'Data written to array should exist in '
                                                                          'written PFB file.')
        del data
        test.close()
        test_read.close()
        os.remove(('test_write_raw.pfb'))

    def test_create_from_data(self):
        data = np.random.random_sample((31, 49, 50))
        test = PFData(data)
        self.assertEqual(31, test.getNX())
        self.assertEqual(49, test.getNY())
        self.assertEqual(50, test.getNZ())
        self.assertEqual(1, test.getP())
        self.assertEqual(1, test.getQ())
        self.assertEqual(1, test.getR())
        self.assertEqual(0, test.getX())
        self.assertEqual(0, test.getY())
        self.assertEqual(0, test.getZ())
        test.writeFile(('test_write_raw.pfb'))
        test_read = PFData(('test_write_raw.pfb'))
        test_read.loadHeader()
        test_read.loadData()
        self.assertEqual(0, test_read.getX())
        self.assertEqual(0, test_read.getY())
        self.assertEqual(0, test_read.getZ())
        self.assertEqual(31, test_read.getNX())
        self.assertEqual(49, test_read.getNY())
        self.assertEqual(50, test_read.getNZ())
        self.assertEqual(1, test_read.getP())
        self.assertEqual(1, test_read.getQ())
        self.assertEqual(1, test_read.getR())
        test_data = test_read.viewDataArray()
        self.assertIsNone(np.testing.assert_array_equal(data, test_data), 'Data written to array should exist in '
                                                                          'written PFB file.')
        del data
        test.close()
        test_read.close()
        os.remove(('test_write_raw.pfb'))

    def test_view(self):
        data = np.random.random_sample((50, 49, 31))
        test = PFData(data)
        view = test.viewDataArray()
        self.assertTrue(np.array_equal(data, view), 'Data obtained from PFData::viewDataArray must match given data')

    def test_copy(self):
        data = np.random.random_sample((50, 49, 31))
        test = PFData(data)
        copy = test.copyDataArray()
        self.assertTrue(np.array_equal(data, copy), 'Data obtained from PFData::copyDataArray must match given data')

    def test_move(self):
        data = np.random.random_sample((50, 49, 31))
        test = PFData(data)
        move = test.moveDataArray()
        self.assertTrue(np.array_equal(data, move), 'Data obtained from PFData::moveDataArray must match given data')
        self.assertIsNone(test.viewDataArray(), 'Calling PFData::moveDataArray must invalidate the internal data pointer')

if __name__ == "__main__":
    unittest.main()
