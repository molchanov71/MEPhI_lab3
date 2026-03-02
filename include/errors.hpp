#pragma once

#include <chrono>
#include <utility>

using std::chrono::system_clock;
using time_point = system_clock::time_point;

template <>
struct std::hash <system_clock::time_point>
{
    size_t operator ()(system_clock::time_point) const noexcept;
};

class MemoryError : public std::exception
{
protected:
    std::string message, type;
    time_point tp;

public:
    explicit MemoryError(std::string message_,
                         const time_point tp_ = system_clock::now(),
                         std::string type_ = "Memory error") noexcept : message(std::move(message_)),
                                                                        type(std::move(type_)),
                                                                        tp(tp_)
    {
    }

    MemoryError(const MemoryError &other) : message(other.message),
                                            type(other.type),
                                            tp(other.tp)
    {
    }

    MemoryError(MemoryError &&other) noexcept : message(std::move(other.message)),
                                                type(std::move(other.type)),
                                                tp(other.tp)
    {
    }

    [[nodiscard]] time_point getTimePoint() const { return tp; }

    [[nodiscard]] const char *what() const noexcept override
    {
        return message.c_str();
    }

    [[nodiscard]] const std::string &getType() const { return type; }

    [[nodiscard]] virtual std::unique_ptr <MemoryError> createUnique() const
    {
        return std::make_unique <MemoryError>(*this);
    }
};

class IndexError final : public MemoryError
{
public:
    explicit IndexError(const std::string &message_, const time_point tp_ = system_clock::now()) noexcept : MemoryError(
        message_,
        tp_,
        "Index error")
    {
    }

    [[nodiscard]] std::unique_ptr <MemoryError> createUnique() const override
    {
        return std::make_unique <IndexError>(*this);
    }
};

class LeakError final : public MemoryError
{
public:
    explicit LeakError(const std::string &message_, const time_point tp_ = system_clock::now()) noexcept : MemoryError(
        message_,
        tp_,
        "Leak error")
    {
    }

    [[nodiscard]] std::unique_ptr <MemoryError> createUnique() const override
    {
        return std::make_unique <LeakError>(*this);
    }
};

class DoubleFreeError final : public MemoryError
{
public:
    explicit DoubleFreeError(const std::string &message_,
                             const time_point tp_ = system_clock::now()) noexcept : MemoryError(
        message_,
        tp_,
        "Double free error")
    {
    }

    [[nodiscard]] std::unique_ptr <MemoryError> createUnique() const override
    {
        return std::make_unique <DoubleFreeError>(*this);
    }
};

class IdAccessError final : public MemoryError
{
public:
    explicit IdAccessError(const std::string &message_,
                           const time_point tp_ = system_clock::now()) noexcept : MemoryError(
        message_,
        tp_,
        "Id access error")
    {
    }

    [[nodiscard]] std::unique_ptr <MemoryError> createUnique() const override
    {
        return std::make_unique <IdAccessError>(*this);
    }
};

class ReferenceError final : public MemoryError
{
public:
    explicit ReferenceError(const std::string &message_,
                            const time_point tp_ = system_clock::now()) noexcept : MemoryError(
        message_,
        tp_,
        "Reference error")
    {
    }

    [[nodiscard]] std::unique_ptr <MemoryError> createUnique() const override
    {
        return std::make_unique <ReferenceError>(*this);
    }
};

class DoubleDefineError final : public MemoryError
{
public:
    explicit DoubleDefineError(const std::string &message_,
                               const time_point tp_ = system_clock::now()) noexcept : MemoryError(
        message_,
        tp_,
        "Double define error")
    {
    }

    [[nodiscard]] std::unique_ptr <MemoryError> createUnique() const override
    {
        return std::make_unique <DoubleDefineError>(*this);
    }
};
