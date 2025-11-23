#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>

std::vector<std::vector<double>> scalarMultiply(const std::vector<std::vector<double>>& matrix, double scalar) {
  std::vector<std::vector<double>> result(matrix.size(), std::vector<double>(matrix.size(), 0.0));

  for (int i = 0; i < matrix.size(); ++i) {
    for (int j = 0; j < matrix.size(); ++j) {
      result[i][j] = matrix[i][j] * scalar;
    }
  }
  return result;
}

std::vector<std::vector<double>> matrixSquare(const std::vector<std::vector<double>>& matrix) {
  std::vector<std::vector<double>> result(matrix.size(), std::vector<double>(matrix.size(), 0.0));
  
  #pragma omp parallel for collapse(2)
  for (int i = 0; i < matrix.size(); ++i) {
    for (int j = 0; j < matrix.size(); ++j) {
      for (int k = 0; k < matrix.size(); ++k) {
        result[i][j] += matrix[i][k] * matrix[k][j];
      }
    }
  }
  return result;
}

std::vector<std::vector<double>> matrixAddition(const std::vector<std::vector<double>>& A, const std::vector<std::vector<double>>& B) {
  if (A.size() != B.size() || A.empty() || A[0].size() != B[0].size()) {
    std::cerr << "Matrices have incompatible sizes for addition.\n";
    return std::vector<std::vector<double>>();
  }
  int rows = A.size();
  int cols = A[0].size();
  std::vector<std::vector<double>> result(rows, std::vector<double>(cols, 0.0));

  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      result[i][j] = A[i][j] + B[i][j];
    }
  }
  return result;
}

std::vector<std::vector<double>> calculateExpression(int size, int threads){
  omp_set_num_threads(threads);


  const double scalar = 2.0;

  std::vector<std::vector<double>> B(size, std::vector<double>(size, 10.0));
  std::vector<std::vector<double>> C(size, std::vector<double>(size, 15.0));
  std::vector<std::vector<double>> I(size, std::vector<double>(size, 0.0));
    for (int i = 0; i < size; ++i) {
      I[i][i] = 1.0;
    }


  std::vector<std::vector<double>> B_squared = matrixSquare(B);
  std::vector<std::vector<double>> C_squared = matrixSquare(C);
  std::vector<std::vector<double>> term1 = matrixSquare(matrixAddition(B_squared, scalarMultiply(C_squared, scalar)));
  std::vector<std::vector<double>> term2 = scalarMultiply(I, scalar);
  std::vector<std::vector<double>> A = matrixAddition(term1, term2);
  return A;
}

int main(int argc, char* argv[]) {
  if (argc == 3){
    int size = std::stoi(argv[1]);
    int threads = std::stoi(argv[2]);
    calculateExpression(size, threads);
  }
  else{
    std::cout << "There are not size or/and threads number in the input\n";
  }
  return 0;
}