import unittest
from pathlib import Path
from parflowio.pyParflowio import PFData
import numpy as np
import os


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
		self.assertSequenceEqual((41, 41, 50), test_data.shape, 'test file array should have shape (41,41,50)')
		self.assertAlmostEqual(98.003604098773, test(0, 0, 0), 12, 'valid data in cell (0,0,0)')
		test_data[0, 0, 0] = 1
		test_data[40, 0, 0] = 1
		test_data[21, 1, 2] = 1
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


if __name__ == "__main__":
	unittest.main()
