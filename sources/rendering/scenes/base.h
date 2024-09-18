#pragma once

namespace scenes
{
    /**
     * Базовый виртуальный класс (интерфейс) сцены
     */
    class Base
    {
    public:
        /**
         * Виртуальный деструктор
         */
        virtual ~Base() = default;

        /**
         * Загрузка всех необходимых ресурсов граф. API
         */
        virtual void load() = 0;

        /**
         * Выгрузка всех использованных ресурсов граф. API
         */
        virtual void unload() = 0;

        /**
         * Обновить данные сцены (положения, камеры, скелеты, прочее)
         * @param delta Разница в секундах между кадрами
         */
        virtual void update(float delta) = 0;

        /**
         * Обновить данные UI (добавить элементы управления и их обработку)
         * @param delta Разница в секундах между кадрами
         */
        virtual void update_ui(float delta) = 0;

        /**
         * Все необходимые команды рисования для отображения сцены
         */
        virtual void render() = 0;

        /**
         * Название сцены (используется для вывода на UI)
         * @return Строка
         */
        virtual const char* name() = 0;
    };
}