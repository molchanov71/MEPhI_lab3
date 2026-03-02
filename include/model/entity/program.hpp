#pragma once

#include "model/entity/unit.hpp"
#include "table.hpp"
#include "errors.hpp"
#include <chrono>

/**
 * @brief Интерфейс управления программой элементами памяти
 */
class IUnitManager
{
public:
    /**
     * @brief Создать переменную
     * @param name Имя
     * @param address Адрес
     * @param size Размер
     * @throw std::logic_error Если имя дублируется или превышена квота
     */
    virtual void createVar(std::string name, size_t address, size_t size) = 0;

    /**
     * @brief Создать массив
     * @param name Имя
     * @param address Адрес
     * @param size Полный размер
     * @param el_size Размер одного элемента
     * @throw std::logic_error Если имя дублируется или превышена квота
     */
    virtual void createArr(std::string name, size_t address, size_t size, size_t el_size) = 0;

    /**
     * @brief Есть ли у программы такой собственный элемент
     * @param name Имя искомого элемента
     * @return Есть ли у программы такой собственный элемент
     */
    [[nodiscard]] virtual bool hasOwnUnit(const std::string &name) const = 0;

    /**
     * @brief Геттер таблицы собственных элементов
     * @return Константная ссылка на таблицу собственных элементов
     */
    [[nodiscard]] virtual const Table <std::string, std::unique_ptr <OwnUnit> > &getOwnUnits() const = 0;

    /**
     * @brief Отделить собственный элемент
     * @param name Имя собственного элемента
     * @return Указатель на отделённый элемент
     * @throw IdAccessError Если элемент не найден
     */
    virtual std::unique_ptr <OwnUnit> extractOwnUnit(const std::string &name) = 0;

    virtual ~IUnitManager() = default;
};

/**
 * @brief Интерфейс управления ссылками
 */
class IRefManager
{
public:
    /**
     * @brief Создать ссылку
     * @param unit_name Имя элемента, на который указывает ссылка
     * @param ref_name Имя создаваемой ссылки
     * @throw std::logic_error Если элемент не найден или имя дублируется
     */
    virtual void createRef(const std::string &unit_name, std::string ref_name) = 0;

    /**
     * @brief Удалить ссылку
     * @param ref_name Имя удаляемой ссылки
     * @throw IdAccessError Если ссылка не найдена
     */
    virtual void removeRef(const std::string &ref_name) = 0;

    /**
     * @brief Геттер ссылки
     * @param ref_name Имя искомой ссылки
     * @return Ссылка на ссылку)))
     * @throw std::logic_error Если ссылка не найдена
     */
    [[nodiscard]] virtual Reference &getRef(const std::string &ref_name) = 0;

    /**
     * @brief Геттер ссылки (константный)
     * @param ref_name Имя искомой ссылки
     * @return Константная ссылка на ссылку)))
     * @throw std::logic_error Если ссылка не найдена
     */
    [[nodiscard]] virtual const Reference &getRef(const std::string &ref_name) const = 0;

    /**
     * @brief Есть ли у программы ссылка с указанным именем
     * @param ref_name Имя искомой ссылки
     * @return Есть ли у программы ссылка с указанным именем
     */
    [[nodiscard]] virtual bool hasRef(const std::string &ref_name) const = 0;

    /**
     * @brief Геттер таблицы
     * @return Константная ссылка на таблицу всех ссылок программы
     */
    [[nodiscard]] virtual const Table <std::string, Reference> &getRefs() const = 0;

    virtual ~IRefManager() = default;
};

/**
 * @brief Интерфейс взаимодействия с разделяемыми сегментами
 */
class ISharedManager
{
public:
    /**
     * @brief Получить доступ к разделяемому сегменту
     * @param sptr Указатель на разделяемый сегмент
     */
    virtual void getAccess(std::shared_ptr <AbstractShared> sptr) = 0;

    /**
     * @brief Отделить разделяемый сегмент
     * @param name Имя разделяемого сегмента
     * @throw IdAccessError Если сегмент не найден
     */
    virtual void detachAccess(const std::string &name) = 0;

    /**
     * @brief Есть ли у программы доступ к разделяемому сегменту с данным именем
     * @param name Имя искомого сегмента
     * @return Есть ли у программы доступ к разделяемому сегменту с данным именем
     */
    [[nodiscard]] virtual bool hasShared(const std::string &name) const = 0;

    /**
     * @brief Геттер таблицы всех разделяемых сегментов
     * @return Константную ссылку на таблицу всех разделяемых сегментов
     */
    [[nodiscard]] virtual const Table <std::string, std::weak_ptr <AbstractShared> > &getSharedUnits() const = 0;

    virtual ~ISharedManager() = default;
};

class Program final : public virtual IUnitManager, public virtual IRefManager, public virtual ISharedManager
{
    std::string path; ///< Путь программы
    size_t quota, own_space;
    ///< Квота программы (максимальный суммарный размер собственных элементов) и суммарный размер собственных элементов
    Table <std::string, std::unique_ptr <OwnUnit> > own_units; ///<  Таблица собственных элементов
    Table <std::string, std::weak_ptr <AbstractShared> > shared_units; ///< Таблица разделяемых сегментов
    Table <std::string, Reference> refs; ///< Таблица ссылок

public:
    /**
     * @brief Конструктор
     * @param path_ Путь программы
     * @param quota_ Квота программы
     */
    Program(std::string path_, const size_t quota_) : path(std::move(path_)),
                                                      quota(quota_),
                                                      own_space(0)
    {
    }

    /**
     * @brief Геттер пути программы
     * @return Путь программы
     */
    [[nodiscard]] std::string_view getPath() const { return path; }

    /**
     * @brief Геттер квоты программы
     * @return Квота программы
     */
    [[nodiscard]] size_t getQuota() const { return quota; }

    /**
     * @brief Создать переменную
     * @param name Имя
     * @param address Адрес
     * @param size Размер
     * @throw std::logic_error Если имя дублируется или превышена квота
     */
    void createVar(std::string name, size_t address, size_t size) override;

    /**
     * @brief Создать массив
     * @param name Имя
     * @param address Адрес
     * @param size Полный размер
     * @param el_size Размер одного элемента
     * @throw std::logic_error Если имя дублируется или превышена квота
     */
    void createArr(std::string name, size_t address, size_t size, size_t el_size) override;

    /**
     * @brief Есть ли у программы такой собственный элемент
     * @param name Имя искомого элемента
     * @return Есть ли у программы такой собственный элемент
     */
    [[nodiscard]] bool hasOwnUnit(const std::string &name) const override { return own_units.contains(name); }

    /**
     * @brief Геттер таблицы собственных элементов
     * @return Константная ссылка на таблицу собственных элементов
     */
    [[nodiscard]] const Table <std::string, std::unique_ptr <OwnUnit> > &getOwnUnits() const override
    {
        return own_units;
    }

    /**
     * @brief Создать ссылку
     * @param unit_name Имя элемента, на который указывает ссылка
     * @param ref_name Имя создаваемой ссылки
     * @throw std::logic_error Если элемент не найден или имя дублируется
     */
    void createRef(const std::string &unit_name, std::string ref_name) override;

    /**
     * @brief Удалить ссылку
     * @param ref_name Имя удаляемой ссылки
     * @throw IdAccessError Если ссылка не найдена
     */
    void removeRef(const std::string &ref_name) override;

    /**
     * @brief Геттер ссылки
     * @param ref_name Имя искомой ссылки
     * @return Ссылка на ссылку)))
     * @throw std::logic_error Если ссылка не найдена
     */
    [[nodiscard]] Reference &getRef(const std::string &ref_name) override;

    /**
     * @brief Геттер ссылки (константный)
     * @param ref_name Имя искомой ссылки
     * @return Константная ссылка на ссылку)))
     * @throw std::logic_error Если ссылка не найдена
     */
    [[nodiscard]] const Reference &getRef(const std::string &ref_name) const override;

    /**
     * @brief Есть ли у программы ссылка с указанным именем
     * @param ref_name Имя искомой ссылки
     * @return Есть ли у программы ссылка с указанным именем
     */
    [[nodiscard]] bool hasRef(const std::string &ref_name) const override;

    /**
     * @brief Геттер таблицы
     * @return Константная ссылка на таблицу всех ссылок программы
     */
    [[nodiscard]] const Table <std::string, Reference> &getRefs() const override { return refs; }

    /**
     * @brief Получить доступ к разделяемому сегменту
     * @param sptr Указатель на разделяемый сегмент
     */
    void getAccess(std::shared_ptr <AbstractShared> sptr) override;

    /**
     * @brief Отделить разделяемый сегмент
     * @param name Имя разделяемого сегмента
     * @throw IdAccessError Если сегмент не найден
     */
    void detachAccess(const std::string &name) override;

    /**
     * @brief Есть ли у программы доступ к разделяемому сегменту с данным именем
     * @param name Имя искомого сегмента
     * @return Есть ли у программы доступ к разделяемому сегменту с данным именем
     */
    [[nodiscard]] bool hasShared(const std::string &name) const override { return shared_units.contains(name); }

    /**
     * @brief Геттер таблицы всех разделяемых сегментов
     * @return Константную ссылку на таблицу всех разделяемых сегментов
     */
    [[nodiscard]] const Table <std::string, std::weak_ptr <AbstractShared> > &getSharedUnits() const override
    {
        return shared_units;
    }

    /**
     * @brief Отделить собственный элемент
     * @param name Имя собственного элемента
     * @return Указатель на отделённый элемент
     * @throw IdAccessError Если элемент не найден
     */
    std::unique_ptr <OwnUnit> extractOwnUnit(const std::string &name) override;

    /**
     * @brief Геттер элемента по имени (собственный/разделяемый/ссылка)
     * @return Ссылка на искомый элемент
     * @throw IdAccessError Если элемент не найден
     */
    [[nodiscard]] AbstractUnit &getUnit(const std::string &);

    /**
     * @brief Геттер элемента по имени (собственный/разделяемый/ссылка) (константный)
     * @param name Имя искомого элемента
     * @return Константная ссылка на искомый элемент
     * @throw IdAccessError Если элемент не найден
     */
    [[nodiscard]] const AbstractUnit &getUnit(const std::string &name) const;

    /**
     * @brief Есть ли такое имя среди собственных/разделяемых элементов или ссылок
     * @param name Имя искомого элемента
     * @return Есть ли такое имя среди собственных/разделяемых элементов или ссылок
     */
    [[nodiscard]] bool hasAnyUnit(const std::string &name) const;
};
