#!/bin/bash

# Скрипт для автоматической настройки окружения и запуска замеров производительности

set -e  # Прерывать выполнение при ошибках

echo "=== Начало настройки окружения ==="

# 1. Создание виртуального окружения Python и установка matplotlib
echo "1. Настройка Python окружения..."
if [ ! -d "venv" ]; then
    echo "Создание виртуального окружения..."
    python3 -m venv venv
fi

echo "Активация виртуального окружения и установка зависимостей..."
source venv/bin/activate
pip install --upgrade pip
pip install matplotlib numpy

# 2. Установка hyperfine
echo "2. Установка hyperfine..."
HYPERFINE_VERSION="v1.18.0"
HYPERFINE_ARCHIVE="hyperfine-${HYPERFINE_VERSION}-x86_64-unknown-linux-musl.tar.gz"
HYPERFINE_URL="https://github.com/sharkdp/hyperfine/releases/download/${HYPERFINE_VERSION}/${HYPERFINE_ARCHIVE}"

# Создаем директорию для hyperfine
HYPERFINE_DIR="hyperfine-bin"
mkdir -p "$HYPERFINE_DIR"

# Скачиваем и распаковываем hyperfine
if [ ! -f "$HYPERFINE_DIR/hyperfine" ]; then
    echo "Скачивание hyperfine..."
    wget -q "$HYPERFINE_URL" -O "$HYPERFINE_ARCHIVE"

    echo "Распаковка hyperfine..."
    tar -xzf "$HYPERFINE_ARCHIVE" --strip-components=1 -C "$HYPERFINE_DIR"

    # Удаляем архив
    rm -f "$HYPERFINE_ARCHIVE"
fi

# Добавляем путь к hyperfine в PATH
export PATH="$(pwd)/$HYPERFINE_DIR:$PATH"
echo "Путь к hyperfine добавлен в PATH"

# Проверяем установку
if command -v hyperfine &> /dev/null; then
    echo "Hyperfine успешно установлен: $(hyperfine --version)"
else
    echo "Ошибка: hyperfine не установлен!"
    exit 1
fi

SCRIPTS="scripts"

# 3. Запуск основного скрипта
echo "3. Запуск основного скрипта..."

# Проверяем наличие основных файлов
if [ ! -f "$SCRIPTS/main_script.py" ]; then
    echo "Ошибка: файл main_script.py не найден!"
    exit 1
fi

if [ ! -f "$SCRIPTS/make_measure.py" ]; then
    echo "Ошибка: файл make_measure.py не найден!"
    exit 1
fi

if [ ! -f "$SCRIPTS/make_graphs.py" ]; then
    echo "Ошибка: файл make_graphs.py не найден!"
    exit 1
fi

# Проверяем наличие C++ файлов по умолчанию
SEQ_SOURCE="labA_07.cpp"
PARALLEL_SOURCE="optimized-versions/labA_07_collapse.cpp"
MPI_SOURCE="optimized-versions/labA_07_mpi.cpp"

# Если файлы не существуют, используем переданные аргументы
if [ $# -ge 1 ]; then
    SEQ_SOURCE="$1"
fi

if [ $# -ge 2 ]; then
    PARALLEL_SOURCE="$2"
fi

# Проверяем существование исходных файлов
if [ ! -f "$SEQ_SOURCE" ]; then
    echo "Предупреждение: файл $SEQ_SOURCE не найден"
    echo "Пожалуйста, укажите существующие файлы как аргументы:"
    echo "./setup_and_run.sh <seq_source.cpp> <parallel_source.cpp>"
    exit 1
fi

if [ ! -f "$PARALLEL_SOURCE" ]; then
    echo "Предупреждение: файл $PARALLEL_SOURCE не найден"
    echo "Пожалуйста, укажите существующие файлы как аргументы:"
    echo "./setup_and_run.sh <seq_source.cpp> <parallel_source.cpp>"
    exit 1
fi

# Запускаем основной скрипт
echo "Запуск измерений с файлами:"
echo "Последовательная версия: $SEQ_SOURCE"
echo "Параллельная версия omp: $PARALLEL_SOURCE"
echo "Параллельная версия mpi: $MPI_SOURCE"

python $SCRIPTS/main_script.py "$SEQ_SOURCE" "$PARALLEL_SOURCE" "$MPI_SOURCE"

echo "=== Замеры завершены! ==="
echo "Результаты сохранены в директории 'result/'"
echo "Графики доступны в директории 'result/graphs/'"
