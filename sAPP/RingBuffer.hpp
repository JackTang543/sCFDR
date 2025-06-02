#pragma once
#include <cstddef>   // for std::size_t
#include <utility>   // for std::move()

/**
 * @brief  裸机环境下的单生产者/单消费者环形缓冲区（Header-only）。
 *         增加了一个 _count 计数器，以便 size／empty／full 都是 O(1) 操作。
 * @tparam T         缓冲区元素类型（需可拷贝或可移动），若在中断中用，请确保不会抛异常。
 * @tparam Capacity  用户可存储的最大元素数量（> 0）。实际内部数组长度为 Capacity + 1。
 *
 * 特点：
 *   - 增加了 count 成员，占用额外 sizeof(size_t) 字节，换来更快的 size/empty/full 判断。
 *   - 依然不使用 std::atomic，仅靠 volatile 索引 + 外部关/开中断保证 SPSC 安全。
 *   - push()/pop() 返回 false 表示失败（满/空），返回 true 表示成功。
 *   - 析构时不会自动析构内部所有元素，如 T 有析构需求请自行 pop 并析构。
 */
template <typename T, std::size_t Capacity>
class RingBuffer
{
private:
    // 实际缓冲区长度：多一个槽位用于区分满/空
    static constexpr std::size_t BufferSize = Capacity + 1;

    // 底层存储
    T       _buffer[BufferSize];

    // 头尾索引，volatile 避免编译器优化掉与中断之间的访问
    volatile std::size_t _head  = 0;
    volatile std::size_t _tail  = 0;
    // 额外再存一个计数器，使 size/empty/full 成 O(1)
    volatile std::size_t _count = 0;

    // 计算“下一个”索引（循环回绕）
    static constexpr std::size_t next_index(std::size_t idx) noexcept
    {
        return (idx + 1) % BufferSize;
    }

public:
    RingBuffer() noexcept = default;
    ~RingBuffer() noexcept = default;

    /** 
     * @brief  向缓冲区尾部写入一个元素（拷贝版）。
     * @param  item  要写入的元素引用。
     * @return 若缓冲区已满（count == Capacity）则返回 false；否则写入并返回 true。
     *
     * 注意：若在中断里调用此函数，主循环可能正在同时 pop，因此
     *       在调用前后应自行关/开中断保护（示例中在外部进行）。
     */
    bool push(const T& item) noexcept
    {
        // 如果已经满了，就直接失败
        if (_count == Capacity) {
            return false;
        }

        const std::size_t tail = _tail;
        _buffer[tail] = item;
        _tail = next_index(tail);
        // 更新计数器
        ++_count;
        return true;
    }

    /**
     * @brief  向缓冲区尾部写入一个元素（移动版）。
     * @param  item  要写入的可移动对象。
     * @return 同上。
     */
    bool push(T&& item) noexcept
    {
        if (_count == Capacity) {
            return false;
        }

        const std::size_t tail = _tail;
        _buffer[tail] = std::move(item);
        _tail = next_index(tail);
        ++_count;
        return true;
    }

    /**
     * @brief  从缓冲区头部弹出一个元素并写入 out。
     * @param  out  引用，弹出的元素会拷贝到 out。如果缓冲区空则返回 false 且 out 不被修改。
     * @return 弹出成功返回 true；若空则返回 false。
     *
     * 注意：若在中断里调用此函数，主循环可能正在同时 push，因此
     *       在调用前后应自行关/开中断保护（示例中在外部进行）。
     */
    bool pop(T& out) noexcept
    {
        if (_count == 0) {
            return false;
        }

        const std::size_t head = _head;
        out = _buffer[head];
        _head = next_index(head);
        --_count;
        return true;
    }

    /**
     * @brief  查看队列头部元素但不弹出，empty() 时不要调用。
     * @return T& 引用到头部元素。若 empty() 为 true，行为未定义。
     */
    T& front() noexcept
    {
        return _buffer[_head];
    }

    const T& front() const noexcept
    {
        return _buffer[_head];
    }

    /// @brief  返回缓冲区中当前存储的元素数量（直接读 _count）
    std::size_t size() const noexcept
    {
        return _count;
    }

    /// @brief  用户可存储的最大元素数（Capacity），不计额外“空槽”
    constexpr std::size_t capacity() const noexcept
    {
        return Capacity;
    }

    /// @brief  如果缓冲区没有任何元素，返回 true
    bool empty() const noexcept
    {
        return (_count == 0);
    }

    /// @brief  如果缓冲区已满（count == Capacity），返回 true
    bool full() const noexcept
    {
        return (_count == Capacity);
    }

    /// @brief  清空缓冲区（重置 head/tail/count），但不析构元素
    void clear() noexcept
    {
        _head  = 0;
        _tail  = 0;
        _count = 0;
    }
};
