#include <gtest/gtest.h>
#include <pcgsolver/sparse_matrix.h>
class SparseMatrixTest : public ::testing::Test{
protected:
  SparseMatrixTest() {
    aMatrix = new SparseMatrix<double>(5, 5);
    aMatrix->set_element(0, 0, 0);
    aMatrix->set_element(1, 0, 1);
    aMatrix->set_element(2, 0, 2);
    aMatrix->set_element(3, 0, 3);
    aMatrix->set_element(4, 0, 4);
  }

  ~SparseMatrixTest() {
    delete aMatrix;
  }

  SparseMatrix<double> *aMatrix;

};

TEST_F(SparseMatrixTest, printMatrix) {
  for(unsigned j = 0; j < 5; ++j) {
    for(unsigned i = 0; i < 5; ++i) {
      std::cout << (*aMatrix)(i, j) << " ";
    }
    std::cout << std::endl;
  }
}

TEST_F(SparseMatrixTest, multiply) {
  std::vector<double> v{1, 2, 3, 4, 5};
  std::vector<double> r;
  multiply((*aMatrix), v, r);
  for(unsigned i = 0; i < 5; ++i) {
    std::cout << r[i] << " ";
  }
}
