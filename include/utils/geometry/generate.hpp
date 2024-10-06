#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

namespace utils::geometry
{
    /**
     * Битовая маска для указания необходимых аттрибутов геометрии
     */
    enum EAttrBit : unsigned
    {
        POSITION    = 1u << 0,
        UV          = 1u << 1,
        NORMAL      = 1u << 2,
        COLOR       = 1u << 3
    };

    /**
     * Генерация квадрата обращенного в сторону камеры/экрана
     * @tparam V Тип вершины
     * @param size Размер стороны квадрата
     * @param req_attr Необходимые аттрибуты
     * @param pos_offset Сдвиг в структуре для аттрибута "положения"
     * @param uv_offset Сдвиг в структуре для аттрибута "uv"
     * @param normal_offset Сдвиг в структуре для аттрибута "нормаль"
     * @param color_offset Сдвиг а структуре для аттрибута "цвет"
     * @param out_indices Указатель на массив индексов
     * @return Массив вершин
     */
    template<typename V>
    inline std::vector<V> gen_quad(float size, unsigned req_attr,
                                   size_t pos_offset = 0,
                                   size_t uv_offset = 0,
                                   size_t normal_offset = 0,
                                   size_t color_offset = 0,
                                   std::vector<GLuint>* out_indices = nullptr)
    {
        std::vector<glm::vec3> positions = {
                {-(size/2.0f), -(size/2.0f), 0.0f},
                {-(size/2.0f), (size/2.0f), 0.0f},
                {(size/2.0f), (size/2.0f), 0.0f},
                {(size/2.0f), -(size/2.0f), 0.0f}
        };
        std::vector<glm::vec2> uvs = {
                {0.0f, 0.0f},
                {0.0f, 1.0f},
                {1.0f, 1.0f},
                {1.0f, 0.0f}
        };
        std::vector<glm::vec3> normals = {
                {0.0f, 0.0f, 1.0f},
                {0.0f, 0.0f, 1.0f},
                {0.0f, 0.0f, 1.0f},
                {0.0f, 0.0f, 1.0f}
        };
        std::vector<glm::vec3> colors = {
                {1.0f, 0.0f,0.0f},
                {0.0f, 1.0f,0.0f},
                {0.0f, 0.0f,1.0f},
                {1.0f, 1.0f,0.0f}
        };

        std::vector<V> vertices(4);
        for(size_t i = 0; i < vertices.size(); i++)
        {
            auto& v = vertices[i];
            auto* pos = (glm::vec3*)((char*)(&(v)) + pos_offset);
            auto* uv = (glm::vec2*)((char*)(&(v)) + uv_offset);
            auto* normal = (glm::vec3*)((char*)(&(v)) + normal_offset);
            auto* color = (glm::vec3*)((char*)(&(v)) + color_offset);

            if(req_attr & EAttrBit::POSITION) *pos = positions[i];
            if(req_attr & EAttrBit::UV) *uv = uvs[i];
            if(req_attr & EAttrBit::NORMAL) *normal = normals[i];
            if(req_attr & EAttrBit::COLOR) *color = colors[i];
        }

        if(out_indices)
        {
            *out_indices = {0,1,2, 2,3,0};
        }

        return vertices;
    }

    /**
     * Генерация куба
     * @tparam V Тип вершины
     * @param size Размер стороны куба
     * @param req_attr Необходимые аттрибуты
     * @param pos_offset Сдвиг в структуре для аттрибута "положения"
     * @param uv_offset Сдвиг в структуре для аттрибута "uv"
     * @param normal_offset Сдвиг в структуре для аттрибута "нормаль"
     * @param color_offset Сдвиг а структуре для аттрибута "цвет"
     * @param out_indices Указатель на массив индексов
     * @return Массив вершин
     */
    template<typename V>
    inline std::vector<V> gen_cube(float size, unsigned req_attr,
                                   size_t pos_offset = 0,
                                   size_t uv_offset = 0,
                                   size_t normal_offset = 0,
                                   size_t color_offset = 0,
                                   std::vector<GLuint>* out_indices = nullptr)
    {
        std::vector<glm::vec3> positions = {
                {-(size/2.0f), -(size/2.0f), (size/2.0f)},
                {-(size/2.0f), (size/2.0f), (size/2.0f)},
                {(size/2.0f), (size/2.0f), (size/2.0f)},
                {(size/2.0f), -(size/2.0f), (size/2.0f)},

                {(size/2.0f), -(size/2.0f), (size/2.0f)},
                {(size/2.0f), (size/2.0f), (size/2.0f)},
                {(size/2.0f), (size/2.0f), -(size/2.0f)},
                {(size/2.0f), -(size/2.0f), -(size/2.0f)},

                {(size/2.0f), -(size/2.0f), -(size/2.0f)},
                {(size/2.0f), (size/2.0f), -(size/2.0f)},
                {-(size/2.0f), (size/2.0f), -(size/2.0f)},
                {-(size/2.0f), -(size/2.0f), -(size/2.0f)},

                {-(size/2.0f), -(size/2.0f), -(size/2.0f)},
                {-(size/2.0f), (size/2.0f), -(size/2.0f)},
                {-(size/2.0f), (size/2.0f), (size/2.0f)},
                {-(size/2.0f), -(size/2.0f), (size/2.0f)},

                {-(size/2.0f), (size/2.0f), (size/2.0f)},
                {-(size/2.0f), (size/2.0f), -(size/2.0f)},
                {(size/2.0f), (size/2.0f), -(size/2.0f)},
                {(size/2.0f), (size/2.0f), (size/2.0f)},

                {-(size/2.0f), -(size/2.0f), -(size/2.0f)},
                {-(size/2.0f), -(size/2.0f), (size/2.0f)},
                {(size/2.0f), -(size/2.0f), (size/2.0f)},
                {(size/2.0f), -(size/2.0f), -(size/2.0f)},
        };
        std::vector<glm::vec2> uvs = {
                {0.0f, 0.0f},
                {0.0f, 1.0f},
                {1.0f, 1.0f},
                {1.0f, 0.0f},

                {0.0f, 0.0f},
                {0.0f, 1.0f},
                {1.0f, 1.0f},
                {1.0f, 0.0f},

                {0.0f, 0.0f},
                {0.0f, 1.0f},
                {1.0f, 1.0f},
                {1.0f, 0.0f},

                {0.0f, 0.0f},
                {0.0f, 1.0f},
                {1.0f, 1.0f},
                {1.0f, 0.0f},

                {0.0f, 0.0f},
                {0.0f, 1.0f},
                {1.0f, 1.0f},
                {1.0f, 0.0f},

                {0.0f, 0.0f},
                {0.0f, 1.0f},
                {1.0f, 1.0f},
                {1.0f, 0.0f}
        };
        std::vector<glm::vec3> normals = {
                {0.0f, 0.0f, 1.0f},
                {0.0f, 0.0f, 1.0f},
                {0.0f, 0.0f, 1.0f},
                {0.0f, 0.0f, 1.0f},

                {1.0f, 0.0f, 0.0f},
                {1.0f, 0.0f, 0.0f},
                {1.0f, 0.0f, 0.0f},
                {1.0f, 0.0f, 0.0f},

                {0.0f, 0.0f, -1.0f},
                {0.0f, 0.0f, -1.0f},
                {0.0f, 0.0f, -1.0f},
                {0.0f, 0.0f, -1.0f},

                {-1.0f, 0.0f, 0.0f},
                {-1.0f, 0.0f, 0.0f},
                {-1.0f, 0.0f, 0.0f},
                {-1.0f, 0.0f, 0.0f},

                {0.0f, 1.0f, 0.0f},
                {0.0f, 1.0f, 0.0f},
                {0.0f, 1.0f, 0.0f},
                {0.0f, 1.0f, 0.0f},

                {0.0f, -1.0f, 0.0f},
                {0.0f, -1.0f, 0.0f},
                {0.0f, -1.0f, 0.0f},
                {0.0f, -1.0f, 0.0f}
        };
        std::vector<glm::vec3> colors = {
        };

        std::vector<V> vertices(24);
        for(size_t i = 0; i < vertices.size(); i++)
        {
            auto& v = vertices[i];
            auto* pos = (glm::vec3*)((char*)(&(v)) + pos_offset);
            auto* uv = (glm::vec2*)((char*)(&(v)) + uv_offset);
            auto* normal = (glm::vec3*)((char*)(&(v)) + normal_offset);
            auto* color = (glm::vec3*)((char*)(&(v)) + color_offset);

            if(req_attr & EAttrBit::POSITION) *pos = positions[i];
            if(req_attr & EAttrBit::UV) *uv = uvs[i];
            if(req_attr & EAttrBit::NORMAL) *normal = normals[i];
            if(req_attr & EAttrBit::COLOR) *color = {0.0f, 0.0f, 0.0f};
        }

        if(out_indices)
        {
            *out_indices = {
                    0,1,2, 2,3,0,
                    4,5,6, 6,7,4,
                    8,9,10, 10,11,8,
                    12,13,14, 14,15,12,
                    16,17,18, 18,19,16,
                    20,21,22, 22,23,20
            };
        }

        return vertices;
    }
}