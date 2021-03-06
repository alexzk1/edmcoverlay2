#pragma once
#include <string>
#include <iostream>
#include <list>
#include "json.hpp"

namespace draw_task
{
    using json = nlohmann::json;

    enum class drawmode_t
    {
        idk,
        text,
        shape,
    };

    struct drawitem_t
    {
        drawmode_t drawmode{drawmode_t::idk};
        // common
        int x{0};
        int y{0};
        std::string color;

        struct drawtext_t
        {
            // text
            std::string text;
            std::string size;
        } text;

        struct drawshape_t
        {
            // shape
            std::string shape;
            std::string fill;
            int w{0};
            int h{0};
            json vect;
        } shape;
    };

    using draw_list = std::list<drawitem_t>;

    /* text message: id, text, color, x, y, ttl, size
    * shape message: id, shape, color, fill, x, y, w, h, ttl
    * color: "red", "yellow", "green", "blue", "#rrggbb"
    * shape: "rect"
    * size: "normal", "large"
    */
    inline draw_list parseJsonString(const std::string& src)
    {
        //hate chained IFs, lets do it more readable....
#define FUNC_PARAMS const json& node, drawitem_t& drawitem
#define LHDR [](FUNC_PARAMS)->void
#define NINT node.get<int>()
#define NSTR node.get<std::string>()
        const static std::map<std::string, std::function<void(FUNC_PARAMS)>> processors =
        {
            {"x", LHDR{drawitem.x = NINT;}},
            {"y", LHDR{drawitem.y = NINT;}},
            {"color", LHDR{drawitem.color = NSTR;}},
            {"text", LHDR{drawitem.drawmode = drawmode_t::text; drawitem.text.text = NSTR;}},
            {"size", LHDR{drawitem.drawmode = drawmode_t::text; drawitem.text.size = NSTR;}},
            {"shape", LHDR{drawitem.drawmode = drawmode_t::shape; drawitem.shape.shape = NSTR;}},
            {"fill", LHDR{drawitem.drawmode = drawmode_t::shape; drawitem.shape.fill = NSTR;}},
            {"w", LHDR{drawitem.drawmode = drawmode_t::shape; drawitem.shape.w = NINT;}},
            {"h", LHDR{drawitem.drawmode = drawmode_t::shape; drawitem.shape.h = NINT;}},
            {"vector", LHDR{drawitem.drawmode = drawmode_t::shape; drawitem.shape.vect = node;}}
        };
#undef NINT
#undef NSTR
#undef LHDR
#undef FUNC_PARAMS

        draw_list res;
        if (!src.empty())
        {
            const auto jsrc = json::parse(src);
            for (const auto& arr_elem : jsrc)
            {
                drawitem_t drawitem;
                for (const auto& kv : arr_elem.items())
                {

                    //std::cout << "Key: [" << kv.key() << "]" << std::endl;

                    const auto it = processors.find(kv.key());
                    if (it != processors.end())
                    {
                        const auto prev_mode  = drawitem.drawmode;
                        it->second(kv.value(), drawitem);
                        if (prev_mode != drawmode_t::idk && drawitem.drawmode != prev_mode)
                        {
                            std::cout << "Mode was double switched text/shape in the same JSON. Ignoring."  << std::endl;
                            drawitem.drawmode = drawmode_t::idk;
                            break;
                        }
                    }
                    else
                        std::cout << "bad key: " << kv.key() << std::endl;
                }
                if (drawitem.drawmode != draw_task::drawmode_t::idk)
                    res.push_back(std::move(drawitem));
            }
        }
        return res;
    }

    //generates lines (x1;y1)-(x2;y2) and calls user callback with it
    //to avoid copy-paste of code for different output devices
    template <class Callback>
    inline bool ForEachVectorPointsPair(const drawitem_t& src, const Callback& func)
    {
        if (src.drawmode == draw_task::drawmode_t::shape && src.shape.shape == "vect")
        {
            constexpr static int UNINIT_COORD = std::numeric_limits<int>::max();
            int x1 = UNINIT_COORD, y1 = UNINIT_COORD, x2 = UNINIT_COORD, y2 = UNINIT_COORD;
            for (const auto& node_ : src.shape.vect.items())
            {
                // node_ is a point
                const auto& val = node_.value();
                int x, y;
                try
                {
                    x = val["x"].get<int>();
                    y = val["y"].get<int>();
                }
                catch (std::exception& e)
                {
                    std::cerr << "Json-point parse failed with message: " << e.what() << std::endl;
                    break;
                }
                catch (...)
                {
                    std::cerr << "Json-point parse failed with uknnown reason." << std::endl;
                    break;
                }

                if (x1 == UNINIT_COORD)
                {
                    x1 = x;
                    y1 = y;
                    continue;
                }
                if (x2 == UNINIT_COORD)
                {
                    x2 = x;
                    y2 = y;
                    func(x1, y1, x2, y2);
                    continue;
                }
                x1 = x2;
                y1 = y2;
                x2 = x;
                y2 = y;
                func(x1, y1, x2, y2);
            }
            return true;
        }
        return false;
    }
}
