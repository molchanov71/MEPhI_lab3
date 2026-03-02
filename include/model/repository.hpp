#pragma once

#include "model/entity/program.hpp"
#include "table.hpp"
#include "errors.hpp"

class AbstractShared;
class AsbtractUnit;

/**
 * @brief Класс одного байта (хранит в себе значение и флаг, обозначающий его использование)
 */
class Byte
{
    uint8_t value; ///< Значение байта
    bool busy; ///< Владеет ли какая-либо переменная этим байтом

public:
    /**
     * @brief Пустой конструктор (инициализирует байт по умолчанию)
     */
    Byte() : value(0),
             busy(false)
    {
    }

    /**
     * @brief Геттер флага байта
     * @return Владеет ли какая-либо переменная этим байтом
     */
    [[nodiscard]] bool isBusy() const { return busy; }

    /**
     * @brief Выдать этот байт программе (ставит флаг в значение true)
     * @throw std::runtime_error Если байт уже выдан
     */
    void grant();

    /**
     * @brief Геттер значения
     * @return Значение байта
     * @throw std::logic_error Если байт не используется (флаг false)
     */
    [[nodiscard]] uint8_t read() const;

    /**
     * @brief Сеттер значения
     * @param new_value Новое значение
     *  @throw std::logic_error Если байт не используется (флаг false)
     */
    void write(uint8_t new_value);

    /**
     * @brief Освободить байт (ставит флаг в значение false)
     * @throw DoubleFreeError Если байт же освобождён
     */
    void detach();
};

/**
 * @brief Интерфейс репозитория
 */
class IProgramRepository
{
public:
    /**
     * @brief Добавить программу
     * @param path Путь программы
     * @param prog Указатель на программу
     * @throw std::logic_error Если такой путь уже есть или квота программы слишком большая
     */
    virtual void addProgram(std::string path, std::unique_ptr <Program> prog) = 0;

    /**
     * @brief Добавить разделяемый сегмент
     * @param name Имя сегмента
     * @param el Указатель на сегмент
     * @throw std::logic_error Если такой сегмент уже есть
     */
    virtual void addShared(std::string name, std::shared_ptr <AbstractShared> el) = 0;

    /**
     * @brief Добавить информацию об ошибке
     * @param path Путь программы, в которой возникла ошибка
     * @param err Указатель на ошибку
     */
    virtual void addError(const std::string &path, std::unique_ptr <MemoryError> err) = 0;

    /**
     * @brief Геттер программы
     * @param path Путь искомой программы
     * @return Ссылку на Program
     * @throw std::logic_error Если программа не найдена
     */
    [[nodiscard]] virtual Program &getProgram(const std::string &path) = 0;

    /**
     * @brief Геттер программы (константный)
     * @param path Путь искомой программы
     * @return Константную ссылку на Program
     * @throw std::logic_error Если программа не найдена
     */
    [[nodiscard]] virtual const Program &getProgram(const std::string &path) const = 0;

    /**
     * @brief Геттер разделяемого сегмента
     * @param name Имя искомого мегмента
     * @return Указатель на AbstractShared
     * @throw std::logic_error Если сегмент не найден
     */
    [[nodiscard]] virtual std::shared_ptr <AbstractShared> getShared(const std::string &name) = 0;

    /**
     * @brief Геттер разделяемого сегмента (константный)
     * @param name Имя искомого мегмента
     * @return Указатель на AbstractShared
     * @throw std::logic_error Если сегмент не найден
     */
    [[nodiscard]] virtual std::shared_ptr <const AbstractShared> getShared(const std::string &name) const = 0;

    /**
     * @brief Удалить программу из репозитория
     * @param path Путь удаляемой программы
     * @throw std::logic_error Если программа не найдена
     */
    virtual void removeProgram(const std::string &path) = 0;

    /**
     * @brief Удалить разделяемый сегмент из репозитория
     * @param name Имя удаляемого сегмента
     * @throw std::logic_error Если сегмент не найден
     */
    virtual void removeShared(const std::string &name) = 0;

    /**
     * @brief Перегрузка оператора []
     * @param idx Индекс искомого байта
     * @return Ссылка на байт
     */
    virtual Byte &operator [](size_t idx) = 0;

    /**
     * @brief Перегрузка оператора [] (константная)
     * @param idx Индекс искомого байта
     * @return Константная ссылка на байт
     */
    virtual const Byte &operator [](size_t idx) const = 0;

    /**
     * @brief Геттер таблицы всех программ
     * @return Константная ссылка на таблицу программ
     */
    [[nodiscard]] virtual const Table <std::string, std::unique_ptr <Program> > &getPrograms() const = 0;

    /**
     * @brief Геттер таблицы всех разделяемых сегментов
     * @return Константная ссылка на таблицу разделяемых сегментов
     */
    [[nodiscard]] virtual const Table <std::string, std::shared_ptr <AbstractShared> > &getSharedElems() const = 0;

    /**
     * @brief Геттер таблицы всех зарегистрированных ошибок
     * @return Константная ссылка на таблицу ошибок
     */
    [[nodiscard]] virtual const Table <std::string, std::vector <std::unique_ptr <MemoryError> > > &getErrors() const =
    0;

    /**
     * @brief Общий объём занимаемой памяти
     * @return Общий объём занимаемой памяти
     */
    [[nodiscard]] virtual size_t getTotalSize() const = 0;

    /**
     * @brief Геттер вектора байтов
     * @return Константная ссылка на вектор байтов
     */
    [[nodiscard]] virtual const std::vector <Byte> &readData() const = 0;

    virtual ~IProgramRepository() = default;
};

class ProgramRepository final : public IProgramRepository
{
    Table <std::string, std::unique_ptr <Program> > programs;
    Table <std::string, std::shared_ptr <AbstractShared> > shared_units;
    Table <std::string, std::vector <std::unique_ptr <MemoryError> > > errors;
    std::vector <Byte> data;
    size_t allocated;

public:
    /**
     * @brief Конструктор
     * @param size Размер создаваемой памяти
     */
    explicit ProgramRepository(const size_t size) : data(size),
                                                    allocated(0)
    {
    }

    /**
     * @brief Добавить программу
     * @param path Путь программы
     * @param prog Указатель на программу
     * @throw std::logic_error Если такой путь уже есть или квота программы слишком большая
     */
    void addProgram(std::string path, std::unique_ptr <Program> prog) override;

    /**
     * @brief Добавить разделяемый сегмент
     * @param name Имя сегмента
     * @param el Указатель на сегмент
     * @throw std::logic_error Если такой сегмент уже есть
     */
    void addShared(std::string name, std::shared_ptr <AbstractShared> el) override;

    /**
     * @brief Добавить информацию об ошибке
     * @param path Путь программы, в которой возникла ошибка
     * @param err Указатель на ошибку
     */
    void addError(const std::string &path, std::unique_ptr <MemoryError> err) override;

    /**
     * @brief Геттер программы
     * @param path Путь искомой программы
     * @return Ссылку на Program
     * @throw std::logic_error Если программа не найдена
     */
    [[nodiscard]] Program &getProgram(const std::string &path) override;

    /**
     * @brief Геттер программы (константный)
     * @param path Путь искомой программы
     * @return Константную ссылку на Program
     * @throw std::logic_error Если программа не найдена
     */
    [[nodiscard]] const Program &getProgram(const std::string &path) const override;

    /**
     * @brief Геттер разделяемого сегмента
     * @param name Имя искомого мегмента
     * @return Указатель на AbstractShared
     * @throw std::logic_error Если сегмент не найден
     */
    [[nodiscard]] std::shared_ptr <AbstractShared> getShared(const std::string &name) override;

    /**
     * @brief Геттер разделяемого сегмента (константный)
     * @param name Имя искомого мегмента
     * @return Указатель на AbstractShared
     * @throw std::logic_error Если сегмент не найден
     */
    [[nodiscard]] std::shared_ptr <const AbstractShared> getShared(const std::string &name) const override;

    /**
     * @brief Удалить программу из репозитория
     * @param path Путь удаляемой программы
     * @throw std::logic_error Если программа не найдена
     */
    void removeProgram(const std::string &path) override;

    /**
     * @brief Удалить разделяемый сегмент из репозитория
     * @param name Имя удаляемого сегмента
     * @throw std::logic_error Если сегмент не найден
     */
    void removeShared(const std::string &name) override;

    /**
     * @brief Перегрузка оператора []
     * @param idx Индекс искомого байта
     * @return Ссылка на байт
     */
    Byte &operator [](const size_t idx) override { return data[idx]; }

    /**
     * @brief Перегрузка оператора [] (константная)
     * @param idx Индекс искомого байта
     * @return Константная ссылка на байт
     */
    const Byte &operator [](const size_t idx) const override
    {
        return data[idx];
    }

    /**
     * @brief Геттер таблицы всех программ
     * @return Константная ссылка на таблицу программ
     */
    [[nodiscard]] const Table <std::string, std::unique_ptr <Program> > &getPrograms() const override
    {
        return programs;
    }

    /**
     * @brief Геттер таблицы всех разделяемых сегментов
     * @return Константная ссылка на таблицу разделяемых сегментов
     */
    [[nodiscard]] const Table <std::string, std::shared_ptr <AbstractShared> > &getSharedElems() const override
    {
        return shared_units;
    }

    /**
     * @brief Геттер таблицы всех зарегистрированных ошибок
     * @return Константная ссылка на таблицу ошибок
     */
    [[nodiscard]] const Table <std::string, std::vector <std::unique_ptr <MemoryError> > > &getErrors() const override
    {
        return errors;
    }

    /**
     * @brief Общий объём занимаемой памяти
     * @return Общий объём занимаемой памяти
     */
    [[nodiscard]] size_t getTotalSize() const override { return data.size(); }

    /**
     * @brief Геттер вектора байтов
     * @return Константная ссылка на вектор байтов
     */
    [[nodiscard]] const std::vector <Byte> &readData() const override
    {
        return data;
    }
};
