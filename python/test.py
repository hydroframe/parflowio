import unittest
from pathlib import Path
from parflowio.pyParflowio import PFData
import numpy as np
import os
import sys
import hashlib


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


class PFDataClassTests(unittest.TestCase):

    @classmethod
    def setUpClass(cls) -> None:
        cls.local_dir = Path(__file__).parent

    def test_empty_init(self):
        test = PFData()
        retval = test.loadHeader()
        self.assertNotEqual(0, retval, 'Empty class should error loading header')

    def test_bad_filename(self):
        test = PFData('bad_filename_not_exists.pfb')
        retval = test.loadHeader()
        self.assertNotEqual(0, retval, 'Bad filename should error loading header')

    def test_good_filename(self):
        test = PFData((self.local_dir / '../tests/inputs/press.init.pfb').as_posix())
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
        test = PFData((self.local_dir / '../tests/inputs/press.init.pfb').as_posix())
        retval = test.loadHeader()
        self.assertEqual(0, retval, 'should load header of file that exists')
        retval = test.loadData()
        self.assertEqual(0, retval, 'should load data from valid file')
        data = test.getDataAsArray()
        self.assertIsNotNone(data, 'data from object should be available as python object')
        self.assertSequenceEqual((50, 41, 41), data.shape)
        self.assertAlmostEqual(98.003604098773, test(0, 0, 0), 12, 'valid data in cell (0,0,0)')
        self.assertAlmostEqual(97.36460429313328, test(40, 0, 0), 12, 'data in cell (40,0,0)')
        self.assertAlmostEqual(98.0043134691891, test(0, 1, 0), 12, 'data in cell (0, 1, 0)')
        self.assertAlmostEqual(98.00901307022781, test(1, 0, 0), 12, 'data in cell (1, 0, 0)')
        self.assertAlmostEqual(92.61370155558751, test(21, 1, 2), 12, 'data in cell (21, 1, 2)')
        self.assertAlmostEqual(7.98008728357588, test(0, 1, 45), 12, 'data in cell (0, 1, 45)')
        self.assertAlmostEqual(97.30205516102234, test(22, 1, 0), 12, 'valid data in cell (22,1,0)')
        self.assertEqual(test(0, 0, 0), data[0, 0, 0], 'data array and c array match values at (0,0,0)')
        self.assertEqual(test(40, 0, 0), data[0, 0, 40], 'data array and c array match values at (40,0,0)')
        self.assertEqual(test(0, 1, 0), data[0, 1, 0], 'data array and c array match values at (0,1,0)')
        self.assertEqual(test(1, 0, 0), data[0, 0, 1], 'data array and c array match values at (1,0,0)')
        self.assertEqual(test(21, 1, 2), data[2, 1, 21], 'data array and c array match values at (21,1,2)')
        self.assertEqual(test(0, 1, 45), data[45, 1, 0], 'data array and c array match values at (0,1,45)')
        self.assertEqual(test(22, 1, 0), data[0, 1, 22], 'data array and c array match values at (22,1,0)')
        test.close()

    def test_read_write_data(self):
        test = PFData((self.local_dir / '../tests/inputs/press.init.pfb').as_posix())
        retval = test.loadHeader()
        self.assertEqual(0, retval, 'should load header of file that exists')
        retval = test.loadData()
        self.assertEqual(0, retval, 'should load data from valid file')
        retval = test.writeFile((self.local_dir / '../tests/inputs/press.init.pfb.tmp').as_posix())
        self.assertEqual(0, retval, 'should write data from previously loaded file')

        data2 = PFData((self.local_dir / '../tests/inputs/press.init.pfb.tmp').as_posix())
        data2.loadHeader()
        data2.loadData()
        self.assertIsNone(np.testing.assert_array_equal(test.getDataAsArray(),
                                                        data2.getDataAsArray(),
                                                        'should read back same values we wrote'))
        in_file_hash = calculate_sha1_hash((self.local_dir / '../tests/inputs/press.init.pfb').as_posix())
        out_file_hash = calculate_sha1_hash((self.local_dir / '../tests/inputs/press.init.pfb.tmp').as_posix())

        # This assertion (that the files are identical) is failing in Python and in C++
        self.assertEqual(in_file_hash, out_file_hash, 'sha1 hash of input and output files match')

        test.close()
        data2.close()
        os.remove((self.local_dir / '../tests/inputs/press.init.pfb.tmp').as_posix())

    def test_manipulate_data(self):
        test = PFData((self.local_dir / '../tests/inputs/press.init.pfb').as_posix())
        retval = test.loadHeader()
        self.assertEqual(0, retval, 'should load header of file that exists')
        retval = test.loadData()
        self.assertEqual(0, retval, 'should load data from valid file')
        test_data = test.getDataAsArray()
        self.assertSequenceEqual((50, 41, 41), test_data.shape, 'test file array should have shape (50,41,41)')
        self.assertAlmostEqual(98.003604098773, test(0, 0, 0), 12, 'valid data in cell (0,0,0)')
        test_data[0, 0, 0] = 1
        test_data[0, 0, 40] = 1
        test_data[2, 1, 21] = 1
        self.assertEqual(1, test(0, 0, 0), 'data update affects underlying array')
        self.assertEqual(1, test(40, 0, 0), 'data update affects underlying array')
        self.assertEqual(1, test(21, 1, 2), 'data update affects underlying array')
        retval = test.writeFile((self.local_dir / '../tests/inputs/press.init.pfb.tmp').as_posix())
        self.assertEqual(0, retval, 'able to write updated data to output file')
        test_read = PFData((self.local_dir / '../tests/inputs/press.init.pfb.tmp').as_posix())
        test_read.loadHeader()
        test_read.loadData()
        self.assertEqual(1, test_read(0, 0, 0), 'updates to data written to file can be read back')
        self.assertEqual(1, test_read(40, 0, 0), 'updates to data written to file can be read back')
        self.assertEqual(1, test_read(21, 1, 2), 'updates to data written to file can be read back')
        test.close()
        test_read.close()
        os.remove((self.local_dir / '../tests/inputs/press.init.pfb.tmp').as_posix())

    def test_dist_file(self):
        test = PFData((self.local_dir / '../tests/inputs/press.init.pfb').as_posix())
        test.distFile(P=2, Q=2, R=1, outFile=(self.local_dir / '../tests/inputs/press.init.pfb.dist').as_posix())

        dist_file = PFData((self.local_dir / '../tests/inputs/press.init.pfb.dist').as_posix())
        self.assertEqual(0, dist_file.loadHeader(), 'should load distributed file header')
        self.assertEqual(0, dist_file.loadData(), 'should load distributed data')
        self.assertIsNone(np.testing.assert_array_equal(test.getDataAsArray(), dist_file.getDataAsArray()),
                          'should find matching data values in original and distributed files')
        test.close()
        dist_file.close()
        os.remove((self.local_dir / '../tests/inputs/press.init.pfb.dist').as_posix())


if __name__ == "__main__":
    unittest.main()
