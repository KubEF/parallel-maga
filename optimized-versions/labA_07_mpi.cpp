#include <iostream>
#include <vector>
#include <chrono>
#include <mpi.h>

// Умножение матрицы на скаляр (локальная операция)
std::vector<std::vector<double>> scalarMultiply(const std::vector<std::vector<double>>& matrix, double scalar) {
    std::vector<std::vector<double>> result(matrix.size(), std::vector<double>(matrix[0].size(), 0.0));
    
    for (size_t i = 0; i < matrix.size(); ++i) {
        for (size_t j = 0; j < matrix[0].size(); ++j) {
            result[i][j] = matrix[i][j] * scalar;
        }
    }
    return result;
}

// Сложение матриц (локальная операция)
std::vector<std::vector<double>> matrixAddition(const std::vector<std::vector<double>>& A, 
                                                  const std::vector<std::vector<double>>& B) {
    if (A.size() != B.size() || A.empty() || A[0].size() != B[0].size()) {
        std::cerr << "Matrices have incompatible sizes for addition.\n";
        return std::vector<std::vector<double>>();
    }
    
    size_t rows = A.size();
    size_t cols = A[0].size();
    std::vector<std::vector<double>> result(rows, std::vector<double>(cols, 0.0));
    
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            result[i][j] = A[i][j] + B[i][j];
        }
    }
    return result;
}

// Возведение матрицы в квадрат с использованием MPI
std::vector<std::vector<double>> matrixSquare(const std::vector<std::vector<double>>& matrix, 
                                               int rank, int num_procs) {
    int n = matrix.size();
    std::vector<std::vector<double>> result(n, std::vector<double>(n, 0.0));
    
    // Преобразуем матрицу в одномерный массив для MPI
    std::vector<double> matrix_flat(n * n);
    std::vector<double> result_flat(n * n, 0.0);
    
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            matrix_flat[i * n + j] = matrix[i][j];
        }
    }
    
    // Распределяем строки между процессами
    int rows_per_proc = n / num_procs;
    int remainder = n % num_procs;
    
    // Вычисляем начальную и конечную строку для каждого процесса
    int start_row = rank * rows_per_proc + std::min(rank, remainder);
    int local_rows = rows_per_proc + (rank < remainder ? 1 : 0);
    
    // Локальный результат для данного процесса
    std::vector<double> local_result(local_rows * n, 0.0);
    
    // Вычисляем свою часть результата
    for (int i = 0; i < local_rows; ++i) {
        int global_i = start_row + i;
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                sum += matrix_flat[global_i * n + k] * matrix_flat[k * n + j];
            }
            local_result[i * n + j] = sum;
        }
    }
    
    // Собираем результаты со всех процессов
    // Подготовка для MPI_Gatherv
    std::vector<int> recvcounts(num_procs);
    std::vector<int> displs(num_procs);
    
    int offset = 0;
    for (int p = 0; p < num_procs; ++p) {
        int p_rows = rows_per_proc + (p < remainder ? 1 : 0);
        recvcounts[p] = p_rows * n;
        displs[p] = offset;
        offset += recvcounts[p];
    }
    
    MPI_Gatherv(local_result.data(), local_rows * n, MPI_DOUBLE,
                result_flat.data(), recvcounts.data(), displs.data(), MPI_DOUBLE,
                0, MPI_COMM_WORLD);
    
    // Рассылаем результат всем процессам
    MPI_Bcast(result_flat.data(), n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    // Преобразуем обратно в двумерный массив
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            result[i][j] = result_flat[i * n + j];
        }
    }
    
    return result;
}

std::vector<std::vector<double>> calculateExpression(int size, int rank, int num_procs) {
    const double scalar = 2.0;
    
    // Инициализация матриц
    std::vector<std::vector<double>> B(size, std::vector<double>(size, 10.0));
    std::vector<std::vector<double>> C(size, std::vector<double>(size, 15.0));
    std::vector<std::vector<double>> I(size, std::vector<double>(size, 0.0));
    
    for (int i = 0; i < size; ++i) {
        I[i][i] = 1.0;
    }
    
    // Вычисления с использованием MPI
    std::vector<std::vector<double>> B_squared = matrixSquare(B, rank, num_procs);
    std::vector<std::vector<double>> C_squared = matrixSquare(C, rank, num_procs);
    std::vector<std::vector<double>> term1 = matrixSquare(
        matrixAddition(B_squared, scalarMultiply(C_squared, scalar)), 
        rank, num_procs
    );
    std::vector<std::vector<double>> term2 = scalarMultiply(I, scalar);
    std::vector<std::vector<double>> A = matrixAddition(term1, term2);
    
    return A;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    
    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    
    if (argc == 2) {
        int size = std::stoi(argv[1]);
        
        // auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::vector<double>> result = calculateExpression(size, rank, num_procs);
        
        // auto end = std::chrono::high_resolution_clock::now();
        // std::chrono::duration<double> elapsed = end - start;
        
        if (rank == 0) {
            std::cout << "Matrix size: " << size << "x" << size << std::endl;
            std::cout << "Number of processes: " << num_procs << std::endl;
            // std::cout << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;
            
            // Вывод части результата для проверки (опционально)
            // if (size <= 5) {
            // int controlValuesSize = 10;
            // std::cout << "Result matrix A:" << std::endl;
            // for (int i = 0; i < controlValuesSize; ++i) {
            //     for (int j = 0; j < controlValuesSize; ++j) {
            //         std::cout << "["<< i << "][" << j << "] = " << result[i][j] << " ";
            //     }
            //     std::cout << std::endl;
            // }
            // }
        }
    } else {
        if (rank == 0) {
            std::cout << "Usage: mpirun -np <num_procs> " << argv[0] << " <matrix_size>\n";
        }
    }
    
    MPI_Finalize();
    return 0;
}