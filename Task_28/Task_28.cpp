#include <iostream>
#include <random>
#include <memory>
#include <thread>
#include <future>
#include <chrono>

static const size_t ARRAY_SIZE = 1000000u;
static const size_t CUT_OFF = 100000u;
static bool multi_thread = true;

template <typename T>
auto merge(T* arr, size_t begin, size_t middle, size_t end) -> void
{
    auto left_size{middle - begin + 1};
    auto right_size{end - middle};
    std::unique_ptr<T[]> left(new T[left_size]);
    std::unique_ptr<T[]> right(new T[right_size]);

    for (auto i{0u}; i < left_size; ++i)
    {
        left[i] = arr[begin + i];
    }

    for (auto i{0u}; i < right_size; ++i)
    {
        right[i] = arr[middle + 1 + i];
    }

    auto i{0u}, j{0u};
    auto index{begin};
    while (i < left_size && j < right_size)
    {
        if (left[i] < right[j])
            arr[index++] = left[i++];
        else
            arr[index++] = right[j++];
    }
    while (i < left_size)
    {
        arr[index++] = left[i++];
    }
    while (j < right_size)
    {
        arr[index++] = right[j++];
    }
}

template <typename T>
auto mergeSort(T* arr, size_t begin, size_t end) -> void
{
    if (begin >= end) return;
    auto middle = (begin + end) / 2;
    if (multi_thread && middle - begin > CUT_OFF)
    {
        auto f = std::async(std::launch::async, [&]() { mergeSort(arr, begin, middle); });
        mergeSort(arr, middle + 1, end);
    }
    else
    {
        mergeSort(arr, begin, middle);
        mergeSort(arr, middle + 1, end);
    }
    merge(arr, begin, middle, end);
}


auto main() -> int
{
    std::unique_ptr<int[]> arr(new int[ARRAY_SIZE]);

    for (auto i{0u}; i < ARRAY_SIZE; ++i)
    {
        arr.get()[i] = rand();
    }
    auto begin = std::chrono::system_clock::now();

    mergeSort(arr.get(), 0, ARRAY_SIZE - 1);

    auto end = std::chrono::system_clock::now();

    auto duration_beg = begin.time_since_epoch();
    auto millis_beg = std::chrono::duration_cast<std::chrono::milliseconds>(duration_beg).count();

    auto duration_end = end.time_since_epoch();
    auto millis_end = std::chrono::duration_cast<std::chrono::milliseconds>(duration_end).count();

    std::cout << "The time: " << millis_end - millis_beg << " milliseconds" << std::endl;

    return 0;
}