#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#pragma region interfaces

/**
 * @brief Интерфейс физического элемента памяти
*/
class IMemoryUnit
{
public:
    /**
     * @brief Полный размер элемента в памяти
     * @return Размер элемента
     */
    [[nodiscard]] virtual size_t getTotalSize() const = 0;

    /**
     * @brief Адрес элемента в памяти
     * @return Адрес (индекс в массиве байтов)
     */
    [[nodiscard]] virtual size_t getAddress() const = 0;

    /**
     * @brief Тип элемента (в данной работе: Variable/Array/Shared)
     * @return Строка ("Variable"/"Array"/"Shared")
     */
    [[nodiscard]] virtual std::string getType() const = 0;

    /**
     * @brief Сеттер адреса (при дефрагментации)
     */
    virtual void setAddress(size_t) = 0;

    virtual ~IMemoryUnit() = default;
};

/**
 * @brief Интерфейс именованного элемента
 */
class INamed
{
public:
    /**
     * @brief Геттер имени
     * @return std::string, содержащая имя элемента
     */
    [[nodiscard]] virtual const std::string &getName() const = 0;

    virtual ~INamed() = default;
};

/**
 * @brief Интерфейс индексируемого элемента
 */
class IIndexed
{
public:
    /**
     * @brief Геттер размера одного элемента
     * @return Размер одного элемента
     */
    [[nodiscard]] virtual size_t getElSize() const = 0;

    /**
     * @brief Длина массива/разделяемого сегмента
     * @return Количество элементов
     */
    [[nodiscard]] virtual size_t getLen() const = 0;

    virtual ~IIndexed() = default;
};

/**
 * @brief Интерфейс разделяемого сегмента
 */
class IShared
{
public:
    /**
     * @brief Добавить программу в список пользователей сегментом
     */
    virtual void grantAccess(std::string) = 0;

    /**
     * @brief Удалить программу из списка
     */
    virtual void revokeAccess(const std::string &) = 0;

    virtual ~IShared() = default;
};

/**
 * @brief Интерфейс ссылки
 */
class IReference
{
public:
    /**
     * @brief Геттер указателя на элемент
     * @return OwnUnit * - указатель на соответствующий элемент
     */
    [[nodiscard]] virtual class OwnUnit *getPtr() = 0;

    /**
     * @brief Геттер указателя на элемент (константный)
     * @return Константный указатель на соответствующий элемент
     */
    [[nodiscard]] virtual const OwnUnit *getPtr() const = 0;

    /**
     * @brief Действует ссылка или нет
     * @return Является ли ссылка недействительной
     */
    [[nodiscard]] virtual bool detached() const = 0;

    /**
     * @brief Сделать ссылку недействительной
     */
    virtual void detach() = 0;

    virtual ~IReference() = default;
};

#pragma endregion

#pragma region abstract classes

/**
 * @brief Абстрактный класс именованного элемента
 */
class AbstractNamed : public INamed
{
protected:
    std::string name; ///< имя элемента

public:
    /**
     * @brief Конструктор
     * @param name_ имя
     */
    explicit AbstractNamed(std::string name_) : name(std::move(name_)) {}

    /**
     * @brief Геттер имени
     * @return std::string, содержащая имя элемента
     */
    [[nodiscard]] const std::string &getName() const override { return name; }
};

/**
 * @brief Абстрактный класс элемента памяти
 */
class AbstractMemoryUnit : public IMemoryUnit
{
protected:
    size_t address, size; ///< Адрес элемента и его размер

public:
    /**
     * @brief Конструктор
     * @param address_ Адрес
     * @param size_ Размер
     */
    AbstractMemoryUnit(const size_t address_, const size_t size_) : address(address_), size(size_) {}

    /**
     * @brief Адрес элемента в памяти
     * @return Адрес (индекс в массиве байтов)
     */
    [[nodiscard]] size_t getAddress() const override { return address; }

    /**
     * @brief Полный размер элемента в памяти
     * @return Размер элемента
     */
    [[nodiscard]] size_t getTotalSize() const override { return size; }

    /**
     * @brief Сеттер адреса (при дефрагментации)
     * @param new_address Новый адрес
     */
    void setAddress(const size_t new_address) override { address = new_address; }
};

/**
 * @brief Абстрактный класс конкретного элемента: имеет имя и расположение в памяти
 */
class AbstractUnit : public AbstractNamed, public AbstractMemoryUnit
{
public:
    /**
     * @brief Конструктор
     * @param name_ Имя
     * @param address_ Адрес
     * @param size_ Размер
     */
    AbstractUnit(std::string name_, const size_t address_, const size_t size_) : AbstractNamed(std::move(name_)),
                                                                               AbstractMemoryUnit(address_, size_) {}
};

/**
 * @brief Абстрактный класс конкретного элемента, принадлежащего программе
 */
class OwnUnit : public AbstractUnit
{
public:
    /**
     * @brief Конструктор
     * @param name_ Имя
     * @param address_ Адрес
     * @param size_ Размер
     */
    OwnUnit(std::string name_, const size_t address_, const size_t size_) : AbstractUnit(
        std::move(name_), address_, size_) {}
};

/**
 * @brief Абстрактный класс индексируемого элемента
 */
class AbstractIndexed : public IIndexed
{
protected:
    size_t el_size; ///< Размер одного элемента

public:
    /**
     * @brief Конструктор
     * @param el_size_ Размер одного элемента
     */
    explicit AbstractIndexed(const size_t el_size_) : el_size(el_size_) {}

    /**
     * @brief Геттер размера одного элемента
     * @return Размер одного элемента
     */
    [[nodiscard]] size_t getElSize() const override { return el_size; }
};

/**
 * @brief Абстрактный класс разделяемого сегмента (имеет место в памяти, индексируемый и обладает своими свойствами)
 */
class AbstractShared : public AbstractUnit, public AbstractIndexed, public IShared
{
protected:
    std::unordered_set<std::string> owning_programs; ///< Программы, которые пользуются сегментом

public:
    /**
     * @brief Конструктор
     * @param name_ Имя
     * @param address_ Адрес
     * @param size_ Полный размер
     * @param el_size_ Размер одного элемента
     */
    AbstractShared(std::string name_, const size_t address_, const size_t size_,
                   const size_t el_size_) : AbstractUnit(std::move(name_), address_, size_),
                                            AbstractIndexed(el_size_) {}

    /**
     * @brief Добавить программу в список пользователей сегментом
     */
    void grantAccess(std::string) override;

    /**
     * @brief Удалить программу из списка
     */
    void revokeAccess(const std::string &) override;

    /**
     * @brief Геттер множества использующих программ
     * @return Множество использующих программ
     */
    [[nodiscard]] const std::unordered_set<std::string> &getOwningPrograms() const
    {
        return owning_programs;
    }
};

#pragma endregion

#pragma region implementations

/**
 * @brief Конкретный класс одиночной переменной
 */
class Variable final : public OwnUnit
{
public:
    /**
     * @brief Конструктор
     * @param name_ Имя
     * @param address_ Адрес
     * @param size_ Размер
     */
    Variable(std::string name_, const size_t address_, const size_t size_) : OwnUnit(
        std::move(name_), address_, size_) {}

    /**
     * @brief Тип элемента
     * @return Строка "Variable"
     */
    [[nodiscard]] std::string getType() const override { return "Variable"; }
};

/**
 * @brief Конкретный класс массива (принадлежит программе и индексируемый)
 */
class Array final : public OwnUnit, public AbstractIndexed
{
public:
    /**
     * @brief Конструктор
     * @param name_ Имя
     * @param address_ Адрес
     * @param size_ Полный размер
     * @param el_size_ Размер одного элемента
     */
    Array(std::string name_, const size_t address_, const size_t size_,
          const size_t el_size_) : OwnUnit(std::move(name_), address_, size_), AbstractIndexed(el_size_) {}

    /**
     * @brief Длина массива
     * @return Количество элементов
     */
    [[nodiscard]] size_t getLen() const override { return size / el_size; }

    /**
     * @brief Тип элемента
     * @return Строка "Array"
     */
    [[nodiscard]] std::string getType() const override { return "Array"; }
};

/**
 * @brief Разделяемый сегмент (индексируемый и используется несколькими программами)
 */
class Shared final : public AbstractShared
{
public:
    /**
     * @brief Конструктор
     * @param name_ Имя
     * @param address_ Адрес
     * @param size_ Полный размер
     * @param el_size_ Размер одного элемента
     */
    Shared(std::string name_, const size_t address_, const size_t size_, const size_t el_size_) : AbstractShared(
        std::move(name_), address_, size_, el_size_) {}

    /**
     * @brief Длина разделяемого сегмента
     * @return Количество элементов
     */
    [[nodiscard]] size_t getLen() const override { return size / el_size; }

    /**
     * @brief Тип элемента
     * @return Строка "Shared"
     */
    [[nodiscard]] std::string getType() const override { return "Shared element"; }
};

/**
 * @brief Класс ссылки (обладает своим именем)
 */
class Reference final : public IReference, public AbstractNamed
{
    OwnUnit *ptr; ///< Указатель на элемент

public:
    /**
     * @brief Конструктор
     * @param name Имя ссылки
     * @param ptr_ Указатель на элемент
     */
    Reference(std::string name, OwnUnit *ptr_) : AbstractNamed(std::move(name)), ptr(ptr_) {}

    /**
     * @brief Геттер указателя на элемент
     * @return OwnUnit * - указатель на соответствующий элемент
     */
    [[nodiscard]] OwnUnit *getPtr() override { return ptr; }

    /**
     * @brief Геттер указателя на элемент (константный)
     * @return Константный указатель на соответствующий элемент
     */
    [[nodiscard]] const OwnUnit *getPtr() const override { return ptr; }

    /**
     * @brief Действует ссылка или нет
     * @return Является ли ссылка недействительной
     */
    [[nodiscard]] bool detached() const override { return ptr == nullptr; }

    /**
     * @brief Сделать ссылку недействительной
     */
    void detach() override { ptr = nullptr; }
};

#pragma endregion
