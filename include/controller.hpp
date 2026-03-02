#pragma once

#include "errors.hpp"
#include "model/service.hpp"
#include <string>

class IProgramRepository;
class IMemoryService;
class IProgramService;
class ISharedService;
class IIndexedService;
class IUnitService;

/**
 * @brief Интерфейс контроллера (содержит методы всех сервисов, в кажом методе использует нужный сервис)
 */
class IController
{
public:
    /**
     * @brief Дефрагментация
     */
    virtual void defragment() const = 0;

    /**
     * @brief Создать программу
     * @param prog_path Путь программы
     * @param quota Квота программы
     * @throw std::logic_error Если путь дублируется или квота слишком большая
     */
    virtual void createProgram(std::string prog_path, size_t quota) = 0;

    /**
     * @brief Удалить программу
     * @param prog_path Путь удаляемой программы
     * @throw std::logic_error Если программа не найдена
     * @throw LeakError Если у программы на момент удаления оставались собственные элементы
     */
    virtual void destroyProgram(const std::string &prog_path) = 0;

    /**
     * @brief Подсчитать общий объём памяти, используемый программой
     * @param path Путь программы
     * @return Общий объём памяти, используемый программой
     * @throw std::logic_error Если программа не найдена
     */
    [[nodiscard]] virtual size_t getTotalMemoryUsage(const std::string &path) const = 0;

    /**
     * @brief Создать разделяемый сегмент (с выделением памяти)
     * @param unit_name Имя сегмента
     * @param total_size Полный размер сегмента
     * @param el_size Размер одного элемента
     * @throw std::logic_error Если имя дублируется
     */
    virtual void createShared(std::string unit_name, size_t total_size, size_t el_size) = 0;

    /**
     * @brief Предоставить программе доступ к разделяемому сегменту
     * @param prog_path Путь программы
     * @param unit_name Имя сегмента
     * @throw std::logic_error Если программа или сегмент не найдены
     */
    virtual void grantShared(const std::string &prog_path, const std::string &unit_name) = 0;

    /**
     * @brief Отозвать от программы доступ к разделяемому сегменту
     * @param prog_path Путь программы
     * @param unit_name Имя сегмента
     * @throw std::logic_error Если программа или сегмент не найдены
     */
    virtual void revokeShared(const std::string &prog_path, const std::string &unit_name) = 0;

    /**
     * @brief Создать массив (с выделением памяти)
     * @param prog_path Путь программы
     * @param unit_name Имя массива
     * @param total_size Полный размер
     * @param el_size Размер одного элемента
     * @throw std::logic_error Если программа не найдена или total_size не кратен el_size
     */
    virtual void createArray(const std::string &prog_path,
                             std::string unit_name,
                             size_t total_size,
                             size_t el_size) = 0;

    /**
     * @brief Создать ссылку
     * @param prog_path Путь программы
     * @param unit_name Имя переменной, на которую создаётся ссылка
     * @param ref_name Имя ссылки
     * @throw std::logic_error Если программа не найдена или имя ссылки дублируется
     * @throw IdAccessError Если переменная не найдена
     */
    virtual void createRef(const std::string &prog_path, const std::string &unit_name, const std::string &ref_name) = 0;

    /**
     * @brief Удалить ссылку
     * @param prog_path Путь программы
     * @param ref_name Имя ссылки
     * @throw std::logic_error Если программа не найдена
     * @throw IdAccessError Если ссылка не найдена
     */
    virtual void removeRef(const std::string &prog_path, const std::string &ref_name) = 0;

    /**
     * @brief Прочитать данные по индексу
     * @param prog_path Путь программы
     * @param unit_name Имя массива/разделяемого сегмента, к которому имеет доступ программа
     * @param idx Искомый индекс
     * @return Данные по индексу idx
     * @throw IndexError Если idx выходит за рамки массива
     * @throw IdAccessError Если индексируемый элемент не найден
     * @throw std::logic_error Если программа не найдена
     */
    [[nodiscard]] virtual std::vector <uint8_t> readIndex(const std::string &prog_path,
                                                          const std::string &unit_name,
                                                          size_t idx) const = 0;

    /**
     * @brief Записать данные по индексу
     * @param prog_path Путь программы
     * @param unit_name Имя массива/разделяемого сегмента, к которому имеет доступ программа
     * @param idx Искомый индекс
     * @param new_data Новые данные
     * @throw IndexError Если idx выходит за рамки массива
     * @throw IdAccessError Если индексируемый элемент не найден
     * @throw std::logic_error Если программа не найдена или размер new_data не совпадает с размером одного элемента
     */
    virtual void writeIndex(const std::string &prog_path,
                            const std::string &unit_name,
                            size_t idx,
                            const std::vector <uint8_t> &new_data) = 0;

    /**
     * @brief Прочитать данные массива из диапазона по двум индексам
     * @param prog_path Путь программы
     * @param unit_name Имя массива
     * @param begin Индекс начала (включительно)
     * @param end Индекс конца (не включительно)
     * @return Данные из диапазона [begin, end)
     * @throw IndexError Если idx выходит за рамки массива
     * @throw IdAccessError Если индексируемый элемент не найден
     * @throw std::logic_error Если программа не найдена
     */
    [[nodiscard]] virtual std::vector <uint8_t> getArrRange(const std::string &prog_path,
                                                            const std::string &unit_name,
                                                            size_t begin,
                                                            size_t end) const = 0;

    /**
     * @brief Прочитать данные разделяемого сегмента из диапазона по двум индексам
     * @param unit_name Имя разделяемого сегмента
     * @param begin Индекс начала (включительно)
     * @param end Индекс конца (не включительно)
     * @return Данные из диапазона [begin, end)
     * @throw IndexError Если idx выходит за рамки массива
     * @throw IdAccessError Если индексируемый элемент не найден
     * @throw std::logic_error Если программа не найдена
     */
    [[nodiscard]] virtual std::vector <uint8_t> getShRange(const std::string &unit_name, size_t begin, size_t end) const = 0;

    /**
     * @brief Создать переменную
     * @param prog_path Путь программы
     * @param unit_name Имя переменной
     * @param size Размер переменной
     * @throw std::logic_error Если программа не найдена или имя переменной дублируется
     */
    virtual void createVariable(const std::string &prog_path, std::string unit_name, size_t size) = 0;

    /**
     * @brief Геттер элемента памяти (одиночной переменной/массива/разделяемого сегмента)
     * @param prog_path Путь программы
     * @param unit_name Имя элемента
     * @return Константная ссылка на элемент
     * @throw std::logic_error Если программа не найдена
     * @throw IdAccessError Если элемент не найден
     */
    [[nodiscard]] virtual const AbstractUnit &getUnit(const std::string &prog_path, const std::string &unit_name) const
    = 0;

    /**
     * @brief Геттер данных из элемента
     * @param prog_path Путь программы
     * @param unit_name Имя элемента
     * @return Данные элемента
     * @throw std::logic_error Если программа не найдена
     * @throw IdAccessError Если элемент не найден
     */
    [[nodiscard]] virtual std::vector <uint8_t> readUnit(const std::string &prog_path,
                                                         const std::string &unit_name) const = 0;

    /**
     * @brief Запись данных в элемент
     * @param prog_path Путь программы
     * @param unit_name Имя элемента
     * @param new_data Новые данные
     * @throw std::logic_error Если программа не найдена или размер новых данных не совпадает с размером элемента
     * @throw IdAccessError Если элемент не найден
     */
    virtual void writeUnit(const std::string &prog_path,
                           const std::string &unit_name,
                           const std::vector <uint8_t> &new_data) = 0;

    /**
     * @brief Удалить собственный элемент
     * @param prog_path Путь программы
     * @param unit_name Имя элемента
     * @throw std::logic_error Если программа не найдена
     * @throw IdAccessError Если собственный элемент не найден
     */
    virtual void destroyOwnUnit(const std::string &prog_path, const std::string &unit_name) = 0;

    virtual ~IController() = default;
};

/**
 * @brief Контроллер - выполняет операции, связанные с изменением состояния
 */
class Controller final : public IController
{
    IProgramRepository &repo; ///< Ссылка на репозиторий
    IMemoryService &mem_service; ///< Ссылка на сервис по управлению памятью
    IProgramService &prog_service; ///< Ссылка на сервис по управлению программами
    ISharedService &sh_service; ///< Ссылка на сервис по управлению разделяемыми сегментами
    IIndexedService &idx_service; ///< Ссылка на сервис по управлению индексируемыми элементами
    IUnitService &unit_service; ///< Ссылка на сервис по управлению элементами памяти
    IRefService &ref_service; ///< Ссылка на сервис по управлению ссылками

public:
    /**
     * @brief Конструктор
     * @param repo Ссылка на репозиторий
     * @param mem_service Ссылка на сервис по управлению памятью
     * @param prog_service Ссылка на сервис по управлению программами
     * @param sh_service Ссылка на сервис по управлению разделяемыми сегментами
     * @param idx_service Ссылка на сервис по управлению индексируемыми элементами
     * @param unit_service Ссылка на сервис по управлению элементами памяти
     * @param ref_service Ссылка на сервис по управлению ссылками
     */
    Controller(IProgramRepository &repo,
               IMemoryService &mem_service,
               IProgramService &prog_service,
               ISharedService &sh_service,
               IIndexedService &idx_service,
               IUnitService &unit_service,
               IRefService &ref_service);

    /**
     * @brief Создать программу
     * @param prog_path Путь программы
     * @param quota Квота программы
     * @throw std::logic_error Если путь дублируется или квота слишком большая
     */
    void createProgram(std::string prog_path, size_t quota) override;

    /**
     * @brief Удалить программу
     * @param prog_path Путь удаляемой программы
     * @throw std::logic_error Если программа не найдена
     * @throw LeakError Если у программы на момент удаления оставались собственные элементы
     */
    void destroyProgram(const std::string &prog_path) override;

    /**
     * @brief Подсчитать общий объём памяти, используемый программой
     * @param path Путь программы
     * @return Общий объём памяти, используемый программой
     * @throw std::logic_error Если программа не найдена
     */
    [[nodiscard]] size_t getTotalMemoryUsage(const std::string &path) const override;

    /**
     * @brief Создать разделяемый сегмент (с выделением памяти)
     * @param unit_name Имя сегмента
     * @param total_size Полный размер сегмента
     * @param el_size Размер одного элемента
     * @throw std::logic_error Если имя дублируется
     */
    void createShared(std::string unit_name, size_t total_size, size_t el_size) override;

    /**
     * @brief Предоставить программе доступ к разделяемому сегменту
     * @param prog_path Путь программы
     * @param unit_name Имя сегмента
     * @throw std::logic_error Если программа или сегмент не найдены
     */
    void grantShared(const std::string &prog_path, const std::string &unit_name) override;

    /**
     * @brief Отозвать от программы доступ к разделяемому сегменту
     * @param prog_path Путь программы
     * @param unit_name Имя сегмента
     * @throw std::logic_error Если программа или сегмент не найдены
     */
    void revokeShared(const std::string &prog_path, const std::string &unit_name) override;

    /**
     * @brief Создать массив (с выделением памяти)
     * @param prog_path Путь программы
     * @param unit_name Имя массива
     * @param total_size Полный размер
     * @param el_size Размер одного элемента
     * @throw std::logic_error Если программа не найдена или total_size не кратен el_size
     */
    void createArray(const std::string &prog_path, std::string unit_name, size_t total_size, size_t el_size) override;

    /**
     * @brief Создать ссылку
     * @param prog_path Путь программы
     * @param unit_name Имя переменной, на которую создаётся ссылка
     * @param ref_name Имя ссылки
     * @throw std::logic_error Если программа не найдена или имя ссылки дублируется
     * @throw IdAccessError Если переменная не найдена
     */
    void createRef(const std::string &prog_path, const std::string &unit_name, const std::string &ref_name) override;

    /**
     * @brief Удалить ссылку
     * @param prog_path Путь программы
     * @param ref_name Имя ссылки
     * @throw std::logic_error Если программа не найдена
     * @throw IdAccessError Если ссылка не найдена
     */
    void removeRef(const std::string &prog_path, const std::string &ref_name) override;

    /**
     * @brief Прочитать данные по индексу
     * @param prog_path Путь программы
     * @param unit_name Имя массива/разделяемого сегмента, к которому имеет доступ программа
     * @param idx Искомый индекс
     * @return Данные по индексу idx
     * @throw IndexError Если idx выходит за рамки массива
     * @throw IdAccessError Если индексируемый элемент не найден
     * @throw std::logic_error Если программа не найдена
     */
    [[nodiscard]] std::vector <uint8_t> readIndex(const std::string &prog_path,
                                                  const std::string &unit_name,
                                                  size_t idx) const override;

    /**
     * @brief Записать данные по индексу
     * @param prog_path Путь программы
     * @param unit_name Имя массива/разделяемого сегмента, к которому имеет доступ программа
     * @param idx Искомый индекс
     * @param new_data Новые данные
     * @throw IndexError Если idx выходит за рамки массива
     * @throw IdAccessError Если индексируемый элемент не найден
     * @throw std::logic_error Если программа не найдена или размер new_data не совпадает с размером одного элемента
     */
    void writeIndex(const std::string &prog_path,
                    const std::string &unit_name,
                    size_t idx,
                    const std::vector <uint8_t> &new_data) override;

    /**
     * @brief Прочитать данные массива из диапазона по двум индексам
     * @param prog_path Путь программы
     * @param unit_name Имя массива
     * @param begin Индекс начала (включительно)
     * @param end Индекс конца (не включительно)
     * @return Данные из диапазона [begin, end)
     * @throw IndexError Если idx выходит за рамки массива
     * @throw IdAccessError Если индексируемый элемент не найден
     * @throw std::logic_error Если программа не найдена
     */
    [[nodiscard]] std::vector <uint8_t> getArrRange(const std::string &prog_path,
                                                    const std::string &unit_name,
                                                    size_t begin,
                                                    size_t end) const override;

    /**
     * @brief Прочитать данные разделяемого сегмента из диапазона по двум индексам
     * @param unit_name Имя разделяемого сегмента
     * @param begin Индекс начала (включительно)
     * @param end Индекс конца (не включительно)
     * @return Данные из диапазона [begin, end)
     * @throw IndexError Если idx выходит за рамки массива
     * @throw IdAccessError Если индексируемый элемент не найден
     * @throw std::logic_error Если программа не найдена
     */
    [[nodiscard]] std::vector <uint8_t> getShRange(const std::string &unit_name, size_t begin, size_t end) const override;

    /**
     * @brief Создать переменную
     * @param prog_path Путь программы
     * @param unit_name Имя переменной
     * @param size Размер переменной
     * @throw std::logic_error Если программа не найдена или имя переменной дублируется
     */
    void createVariable(const std::string &prog_path, std::string unit_name, size_t size) override;

    /**
     * @brief Геттер элемента памяти (одиночной переменной/массива/разделяемого сегмента)
     * @param prog_path Путь программы
     * @param unit_name Имя элемента
     * @return Константная ссылка на элемент
     * @throw std::logic_error Если программа не найдена
     * @throw IdAccessError Если элемент не найден
     */
    [[nodiscard]] const AbstractUnit &
    getUnit(const std::string &prog_path, const std::string &unit_name) const override;

    /**
     * @brief Геттер данных из элемента
     * @param prog_path Путь программы
     * @param unit_name Имя элемента
     * @return Данные элемента
     * @throw std::logic_error Если программа не найдена
     * @throw IdAccessError Если элемент не найден
     */
    [[nodiscard]] std::vector <uint8_t> readUnit(const std::string &prog_path,
                                                 const std::string &unit_name) const override;

    /**
    * @brief Запись данных в элемент
    * @param prog_path Путь программы
    * @param unit_name Имя элемента
    * @param new_data Новые данные
    * @throw std::logic_error Если программа не найдена или размер новых данных не совпадает с размером элемента
    * @throw IdAccessError Если элемент не найден
    */
    void writeUnit(const std::string &prog_path, const std::string &unit_name, const std::vector <uint8_t> &new_data) override;

    /**
     * @brief Удалить собственный элемент
     * @param prog_path Путь программы
     * @param unit_name Имя элемента
     * @throw std::logic_error Если программа не найдена
     * @throw IdAccessError Если собственный элемент не найден
     */
    void destroyOwnUnit(const std::string &prog_path, const std::string &unit_name) override;

    /**
     * @brief Дефрагментация
     */
    void defragment() const override;
};
