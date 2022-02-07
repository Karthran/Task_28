// DEBUG
#include <iostream>

#include <random>
#include <memory>
#include <thread>
#include <future>
#include <chrono>
#include <mutex>
#include <atomic>

static const size_t ARRAY_SIZE = 1000000u;
static const size_t CUT_OFF = 100000u;
static bool multi_thread = true;
std::atomic<unsigned int> available_thread = 0;

std::mutex av;

auto checkCutOff(size_t amount) -> bool
{
    return amount > CUT_OFF;
}

auto checkHardwareConcurrency(size_t amount = 0) -> bool
{
    //    std::cout << "Thread left: " << available_thread << std::endl;
    return available_thread > 0;
}

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
auto mergeSort(T* arr, size_t begin, size_t end, bool (*pred)(size_t)) -> void
{
    // std::cout << "Thread left: " << available_thread << std::endl;

    if (begin >= end) return;
    auto middle = (begin + end) / 2;
    if (multi_thread && pred(middle - begin))
    {
        auto f = std::async(std::launch::async, [&]() { mergeSort(arr, begin, middle, pred); });
        mergeSort(arr, middle + 1, end, pred);
    }
    else
    {
        mergeSort(arr, begin, middle, pred);
        mergeSort(arr, middle + 1, end, pred);
    }
    merge(arr, begin, middle, end);
}

template <typename T>
auto mergeSort_adaptive(T* arr, size_t begin, size_t end, bool (*pred)(size_t)) -> void
{
//    std::cout << "Thread left: " << available_thread << std::endl;

    if (begin >= end) return;
    auto middle = (begin + end) / 2;

    //av.lock();
    if (multi_thread && pred(middle - begin))
    {
        //std::cout << "Start New Thread : " << available_thread << std::endl;

        --available_thread;
//        std::cout << "Start Thread left: " << available_thread << std::endl;
        //av.unlock();
        auto f = std::async(std::launch::async, [&]() { mergeSort(arr, begin, middle, pred); });
        mergeSort_adaptive(arr, middle + 1, end, pred);
        {
           // std::lock_guard<std::mutex> lock(av);
            //av.lock();
            ++available_thread;
            //av.unlock();
//            std::cout << "Stop Thread left: " << available_thread << std::endl;
        }
    }
    else
    {
        //std::cout << "Work in Current Thread left: " << available_thread << std::endl;

        //av.unlock();
        mergeSort_adaptive(arr, begin, middle, pred);
        mergeSort_adaptive(arr, middle + 1, end, pred);
    }
    merge(arr, begin, middle, end);
}

auto foo() -> void
{
    auto f = std::async(std::launch::async, []() { std::this_thread::sleep_for(std::chrono::seconds(5)); });
    std::cout << "stop" << std::endl;
}

auto main() -> int
{
    // std::cout << "Start" << std::endl;
    // foo();
    // std::cout << "End" << std::endl;

    bool (*cut_off)(size_t) = checkCutOff;
    bool (*concurrency)(size_t) = checkHardwareConcurrency;
    available_thread = std::thread::hardware_concurrency();

    std::cout << available_thread << std::endl;

  //  int arr[ARRAY_SIZE]{};

    std::unique_ptr<int[]> arr(new int[ARRAY_SIZE]);

    for (auto i{0u}; i < ARRAY_SIZE; ++i)
    {
        arr.get()[i] = rand();
    }
    auto begin = std::chrono::system_clock::now();

    mergeSort(arr.get(), 0, ARRAY_SIZE - 1, cut_off);

    auto end = std::chrono::system_clock::now();

    auto duration_beg = begin.time_since_epoch();
    auto millis_beg = std::chrono::duration_cast<std::chrono::milliseconds>(duration_beg).count();

    auto duration_end = end.time_since_epoch();
    auto millis_end = std::chrono::duration_cast<std::chrono::milliseconds>(duration_end).count();

    std::cout << "The time: " << millis_end - millis_beg << " milliseconds" << std::endl;

    return 0;
}