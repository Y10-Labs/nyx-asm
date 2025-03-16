struct Vertex
{
    glm::vec4 v;
    glm::vec3 col;
};

struct Vertex16
{
    glm::i16vec2 v;
    glm::u16vec2 depth;
    uint16_t col_uv;
};

float edgeFunction(const glm::vec4& a, const glm::vec4& b, const glm::vec3& c)
{
    return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
}

int32_t edgeFunction32(const glm::u16vec2& a, const glm::u16vec2& b, const glm::u16vec2& c)
{
    return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
}

void test_raster(sf::Image& img)
{
    glm::mat4 P = glm::perspective(glm::radians(90.0f), 5.0f / 3.0f, 1.0f, 2000.0f);
    glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

    glm::mat4 MVP = P * V;

    std::vector<Vertex> vertex_array = {
        Vertex{{0.0f, 0.0f, 1.0f, 1.0f}, glm::vec3(1.0f, 0.0f, 0.0f)},
        Vertex{{1.0f, 0.0f, 1.0f, 1.0f}, glm::vec3(0.0f, 1.0f, 0.0f)},
        Vertex{{0.0f, 0.0f, 2.0f, 1.0f}, glm::vec3(0.0f, 0.0f, 1.0f)},

        Vertex{{1.0f, 0.0f, 2.0f, 1.0f}, glm::vec3(1.0f, 0.0f, 0.0f)},

        // 2
        Vertex{{-2.0f, -1.01f, 0.0f, 1.0f}, glm::vec3(1.0f, 1.0f, 1.0f)},
        Vertex{{ 2.0f, -1.01f, 0.0f, 1.0f}, glm::vec3(1.0f, 1.0f, 1.0f)},
        Vertex{{-2.0f, -1.01f, 16.0f, 1.0f}, glm::vec3(1.0f, 1.0f, 1.0f)},

        Vertex{{ 2.0f, -1.01f, 16.0f, 1.0f}, glm::vec3(1.0f, 1.0f, 1.0f)},
    };

    size_t triangle_cnt = 4;
    std::vector<size_t> triangle_indices = {
        0, 1, 2,
        1, 3, 2,
        4, 5, 6,
        5, 7, 6
    };

    // transform all vertex
    std::vector<Vertex> tva;
    for (auto& v : vertex_array)
    {
        auto vp = MVP * v.v;
        vp.x /= vp.w;
        vp.y /= vp.w;
        vp.z /= vp.w;
        tva.push_back(Vertex{ vp , v.col });
    }

    size_t imageWidth = 400;
    size_t imageHeight = 240;
    size_t imageSize = imageWidth*imageHeight;

    auto depthBuffer = new float[imageSize];
    img.create(imageWidth, imageHeight, sf::Color::Black);

    for (size_t i = 0; i < imageSize; i++) depthBuffer[i] = 0.0f;

    for (uint32_t i = 0; i < triangle_cnt; ++i) {
        const auto& v0 = tva[triangle_indices[i * 3]];
        const auto& v1 = tva[triangle_indices[i * 3 + 1]];
        const auto& v2 = tva[triangle_indices[i * 3 + 2]];

        auto v0Raster=v0.v, v1Raster=v1.v, v2Raster=v2.v;

        v0Raster.x = (v0Raster.x + 1.0) / 2; v0Raster.x *= imageWidth;
        v0Raster.y = (v0Raster.y + 1.0) / 2; v0Raster.y *= imageHeight;
        v0Raster.z = (-v0Raster.z + 1.0) / 2;

        v1Raster.x = (v1Raster.x + 1.0) / 2; v1Raster.x *= imageWidth;
        v1Raster.y = (v1Raster.y + 1.0) / 2; v1Raster.y *= imageHeight;
        v1Raster.z = (-v1Raster.z + 1.0) / 2;

        v2Raster.x = (v2Raster.x + 1.0) / 2; v2Raster.x *= imageWidth;
        v2Raster.y = (v2Raster.y + 1.0) / 2; v2Raster.y *= imageHeight;
        v2Raster.z = (-v2Raster.z + 1.0) / 2;

        v0Raster.w = 1 / v0Raster.w;
        v1Raster.w = 1 / v1Raster.w;
        v2Raster.w = 1 / v2Raster.w;

        auto st0 = tva[triangle_indices[i * 3]].col;
        auto st1 = tva[triangle_indices[i * 3 + 1]].col;
        auto st2 = tva[triangle_indices[i * 3 + 2]].col;

        st0 *= v0Raster.w, st1 *= v1Raster.w, st2 *= v2Raster.w;

        int16_t xmin = std::min({v0Raster.x, v1Raster.x, v2Raster.x});
        int16_t ymin = std::min({v0Raster.y, v1Raster.y, v2Raster.y});
        int16_t xmax = std::max({v0Raster.x, v1Raster.x, v2Raster.x});
        int16_t ymax = std::max({v0Raster.y, v1Raster.y, v2Raster.y});

        if (xmin > imageWidth || xmax < 0 || ymin > imageHeight || ymax < 0) continue;

        uint32_t x0 = std::max(int32_t(0), (int32_t)(std::floor(xmin)));
        uint32_t x1 = std::min(int32_t(imageWidth) - 1, (int32_t)(std::floor(xmax)));
        uint32_t y0 = std::max(int32_t(0), (int32_t)(std::floor(ymin)));
        uint32_t y1 = std::min(int32_t(imageHeight) - 1, (int32_t)(std::floor(ymax)));

        float area = edgeFunction(v0Raster, v1Raster, v2Raster);

        for (uint32_t y = y0; y <= y1; ++y) {
            for (uint32_t x = x0; x <= x1; ++x) {
                glm::vec3 pixelSample((x + 0.5), (y + 0.5), 0);
                float w0 = edgeFunction(v1Raster, v2Raster, pixelSample);
                float w1 = edgeFunction(v2Raster, v0Raster, pixelSample);
                float w2 = edgeFunction(v0Raster, v1Raster, pixelSample);
                if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                    w0 /= area;
                    w1 /= area;
                    w2 /= area;
                    float norm_d = v0Raster.z * w0 + v1Raster.z * w1 + v2Raster.z * w2;
                    float oneOverZ = v0Raster.w * w0 + v1Raster.w * w1 + v2Raster.w * w2;
                    float z = 1 / oneOverZ;

                    if (norm_d > depthBuffer[y * imageWidth + x]) {
                        depthBuffer[y * imageWidth + x] = norm_d;

                        auto col = (st0 * w0 + st1 * w1 + st2 * w2) * z;

                        //uint8_t red = col.r * 255;
                        //uint8_t green = col.g * 255;
                        //uint8_t blue = col.b * 255;

                        uint8_t red = (i & 0x01) * 255;
                        uint8_t green = (i & 0x02) * 255;
                        uint8_t blue = (i == 0) * 255;

                        // limit colors to 15-bit
                        red &= 0xF8;
                        green &= 0xF8;
                        blue &= 0xF8;

                        img.setPixel(x, y, sf::Color(red, green, blue));
                    }
                }
            }
        }
    }

    img.saveToFile("tmp.png");
}

void test_raster_16bit(sf::Image& img, OGL::Camera& cam)
{
    float near = 1.0f;
    float far = 2000.0f;

    glm::mat4 P = glm::perspective(glm::radians(90.0f), 5.0f / 3.0f, near, far);
    //glm::lookAt(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    glm::mat4 V = cam.Get_View_Matrix(); 

    glm::mat4 MVP = P * V;

    std::vector<Vertex> vertex_array = {
        Vertex{{0.0f, 0.0f, 1.0f, 1.0f}, glm::vec3(1.0f, 0.0f, 0.0f)},
        Vertex{{1.0f, 0.0f, 1.0f, 1.0f}, glm::vec3(0.0f, 1.0f, 0.0f)},
        Vertex{{0.0f, 0.0f, 2.0f, 1.0f}, glm::vec3(0.0f, 0.0f, 1.0f)},

        Vertex{{1.0f, 0.0f, 2.0f, 1.0f}, glm::vec3(1.0f, 0.0f, 0.0f)},

        // 2
        Vertex{{-2.0f, -1.01f, 0.0f, 1.0f}, glm::vec3(1.0f, 1.0f, 1.0f)},
        Vertex{{ 2.0f, -1.01f, 0.0f, 1.0f}, glm::vec3(1.0f, 1.0f, 1.0f)},
        Vertex{{-2.0f, -1.01f, 16.0f, 1.0f}, glm::vec3(1.0f, 1.0f, 1.0f)},

        Vertex{{ 2.0f, -1.01f, 16.0f, 1.0f}, glm::vec3(1.0f, 1.0f, 1.0f)},
    };

    size_t triangle_cnt = 4;
    std::vector<size_t> triangle_indices = {
        0, 1, 2,
        1, 3, 2,
        4, 5, 6,
        5, 7, 6
    };

    size_t imageWidth = 400;
    size_t imageHeight = 240;
    size_t imageSize = imageWidth * imageHeight;

    // transform all vertex
    // done on CPU
    std::vector<Vertex16> tva;
    for (auto& v : vertex_array)
    {
        auto vp = MVP * v.v;

        // convert to NDC Coords
        vp.x /= vp.w;
        vp.y /= vp.w;
        vp.z /= vp.w;

        // viewport transform
        vp.x = ( vp.x + 1.0) / 2; vp.x *= imageWidth;
        vp.y = ( vp.y + 1.0) / 2; vp.y *= imageHeight;
        vp.z = (-vp.z + 1.0) / 2;
        vp.w = near / vp.w;

        // int16 typecast
        glm::i16vec2 pos = glm::i16vec2(vp.x * (1 << 6), vp.y * (1 << 6));
        glm::u16vec2 depth = glm::u16vec2(vp.z * ((uint16_t)1 << 15), vp.w * ((uint16_t)1 << 15));
        uint16_t col_uv = (((uint8_t)(v.col.r * 255) & 0xF8) << 7) | (((uint8_t)(v.col.g * 255) & 0xF8) << 2) | (((uint8_t)(v.col.b * 255) & 0xF8) >> 3);

        tva.push_back(Vertex16{ pos, depth , col_uv });
    }

    auto depthBuffer = new uint16_t[imageSize];
    img.create(imageWidth, imageHeight, sf::Color::Black);

    for (size_t i = 0; i < imageSize; i++) depthBuffer[i] = 0;

    for (uint32_t i = 0; i < triangle_cnt; ++i) {
        const auto& v0 = tva[triangle_indices[i * 3]];
        const auto& v1 = tva[triangle_indices[i * 3 + 1]];
        const auto& v2 = tva[triangle_indices[i * 3 + 2]];

        auto v0Raster = v0.v, v1Raster = v1.v, v2Raster = v2.v;

        auto v0cmp_z = v0.depth.x;
        auto v1cmp_z = v1.depth.x;
        auto v2cmp_z = v2.depth.x;

        auto v0_z = v0.depth.y;
        auto v1_z = v1.depth.y;
        auto v2_z = v2.depth.y;

        auto st0_packed = tva[triangle_indices[i * 3 + 0]].col_uv;
        auto st1_packed = tva[triangle_indices[i * 3 + 1]].col_uv;
        auto st2_packed = tva[triangle_indices[i * 3 + 2]].col_uv;

        auto st0 = glm::u32vec3((st0_packed & 0x7C00) >> 7, (st0_packed & 0x03E0) >> 2, (st0_packed & 0x001F) << 3);
        auto st1 = glm::u32vec3((st1_packed & 0x7C00) >> 7, (st1_packed & 0x03E0) >> 2, (st1_packed & 0x001F) << 3);
        auto st2 = glm::u32vec3((st2_packed & 0x7C00) >> 7, (st2_packed & 0x03E0) >> 2, (st2_packed & 0x001F) << 3);

        auto tmp_col = st0;

        // divide by w coord, 
        st0.x = (st0.x << 15) / v0_z; st0.y = (st0.y << 15) / v0_z; st0.z = (st0.z << 15) / v0_z;
        st1.x = (st1.x << 15) / v1_z; st1.y = (st1.y << 15) / v1_z; st1.z = (st1.z << 15) / v1_z;
        st2.x = (st2.x << 15) / v2_z; st2.y = (st2.y << 15) / v2_z; st2.z = (st2.z << 15) / v2_z;

        int16_t xmin = std::min({ v0Raster.x >> 6, v1Raster.x >> 6, v2Raster.x >> 6 });
        int16_t ymin = std::min({ v0Raster.y >> 6, v1Raster.y >> 6, v2Raster.y >> 6 });
        int16_t xmax = std::max({ v0Raster.x >> 6, v1Raster.x >> 6, v2Raster.x >> 6 });
        int16_t ymax = std::max({ v0Raster.y >> 6, v1Raster.y >> 6, v2Raster.y >> 6 });

        if (xmin > imageWidth || xmax < 0 || ymin > imageHeight || ymax < 0) continue;

        int32_t x0 = std::max(int32_t(0), (int32_t)(std::floor(xmin)));
        int32_t x1 = std::min(int32_t(imageWidth) - 1, (int32_t)(std::floor(xmax)));
        int32_t y0 = std::max(int32_t(0), (int32_t)(std::floor(ymin)));
        int32_t y1 = std::min(int32_t(imageHeight) - 1, (int32_t)(std::floor(ymax)));

        int32_t area = edgeFunction32(v0Raster, v1Raster, v2Raster);

        glm::i16vec2 pixelSample((x0 << 6) + (1 << 5), (y0 << 6) + (1 << 5));
        int32_t w0 = ((int64_t)edgeFunction32(v1Raster, v2Raster, pixelSample) << 24) / area;
        int32_t w1 = ((int64_t)edgeFunction32(v2Raster, v0Raster, pixelSample) << 24) / area;
        int32_t w2 = ((int64_t)edgeFunction32(v0Raster, v1Raster, pixelSample) << 24) / area;

        int32_t w0_dy = ((int64_t)(v2Raster.x - v1Raster.x) << 30) / area;
        int32_t w1_dy = ((int64_t)(v0Raster.x - v2Raster.x) << 30) / area;
        int32_t w2_dy = ((int64_t)(v1Raster.x - v0Raster.x) << 30) / area;

        int32_t w0_dx = ((int64_t)(v2Raster.y - v1Raster.y) << 30) / area;
        int32_t w1_dx = ((int64_t)(v0Raster.y - v2Raster.y) << 30) / area;
        int32_t w2_dx = ((int64_t)(v1Raster.y - v0Raster.y) << 30) / area;

        for (uint32_t y = y0, k = 0; y <= y1; ++y, k++) {
            if (k & 1)
            {
                for (int32_t x = x1; x >= x0; x--) {
                    //glm::i16vec2 pixelSample((x << 6) + (1 << 5), (y << 6) + (1 << 5));
                    //int32_t w0_ref = ((int64_t)edgeFunction32(v1Raster, v2Raster, pixelSample) << 24) / area;
                    //int32_t w1_ref = ((int64_t)edgeFunction32(v2Raster, v0Raster, pixelSample) << 24) / area;
                    //int32_t w2_ref = ((int64_t)edgeFunction32(v0Raster, v1Raster, pixelSample) << 24) / area;
                    if (x < 0 || y < 0)
                    {
                        std::cout << x << ", " << x0 << ", " << x1 << "\n";
                    }
                    if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                        //int32_t tw0 = ((int64_t)w0 << 16) / area;
                        //int32_t tw1 = ((int64_t)w1 << 16) / area;
                        //int32_t tw2 = ((int64_t)w2 << 16) / area;

                        int32_t norm_d = (int32_t)(v0cmp_z * (w0 >> 8)) + (int32_t)(v1cmp_z * (w1 >> 8)) + (int32_t)(v2cmp_z * (w2 >> 8));
                        norm_d = norm_d >> 16;

                        if (norm_d > depthBuffer[y * imageWidth + x]) {
                            //depthBuffer[y * imageWidth + x] = norm_d;

                            //float oneOverZ = ((uint32_t)w0 << 15)/v0_z + ((uint32_t)w1 << 15) / v1_z + ((uint32_t)w2 << 15) / v2_z;

                            //auto col = (st0 * w0 + st1 * w1 + st2 * w2) * z;

                            //uint8_t red = tmp_col.r;
                            //uint8_t green = tmp_col.g;
                            //uint8_t blue = tmp_col.b;

                            uint8_t red = (i & 0x01) * 255;
                            uint8_t green = (i & 0x02) * 255;
                            uint8_t blue = (i == 0) * 255;

                            // limit colors to 15-bit
                            red &= 0xF8;
                            green &= 0xF8;
                            blue &= 0xF8;

                            img.setPixel(x, y, sf::Color(red, green, blue));
                        }
                    }
                    w0 -= w0_dx;
                    w1 -= w1_dx;
                    w2 -= w2_dx;
                }
                w0 += w0_dx;
                w1 += w1_dx;
                w2 += w2_dx;
            }
            else
            {
                for (int32_t x = x0; x <= x1; ++x) {
                    //glm::i16vec2 pixelSample((x << 6) + (1 << 5), (y << 6) + (1 << 5));
                    //int32_t w0_ref = ((int64_t)edgeFunction32(v1Raster, v2Raster, pixelSample) << 24) / area;
                    //int32_t w1_ref = ((int64_t)edgeFunction32(v2Raster, v0Raster, pixelSample) << 24) / area;
                    //int32_t w2_ref = ((int64_t)edgeFunction32(v0Raster, v1Raster, pixelSample) << 24) / area;
                    if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                        //int32_t tw0 = ((int64_t)w0 << 16) / area;
                        //int32_t tw1 = ((int64_t)w1 << 16) / area;
                        //int32_t tw2 = ((int64_t)w2 << 16) / area;

                        int32_t norm_d = (int32_t)(v0cmp_z * (w0 >> 8)) + (int32_t)(v1cmp_z * (w1 >> 8)) + (int32_t)(v2cmp_z * (w2 >> 8));
                        norm_d = norm_d >> 16;

                        if (norm_d > depthBuffer[y * imageWidth + x]) {
                            depthBuffer[y * imageWidth + x] = norm_d;

                            //float oneOverZ = ((uint32_t)w0 << 15)/v0_z + ((uint32_t)w1 << 15) / v1_z + ((uint32_t)w2 << 15) / v2_z;

                            //auto col = (st0 * w0 + st1 * w1 + st2 * w2) * z;

                            //uint8_t red = tmp_col.r;
                            //uint8_t green = tmp_col.g;
                            //uint8_t blue = tmp_col.b;

                            uint8_t red = (i & 0x01) * 255;
                            uint8_t green = (i & 0x02) * 255;
                            uint8_t blue = (i == 0) * 255;

                            // limit colors to 15-bit
                            red &= 0xF8;
                            green &= 0xF8;
                            blue &= 0xF8;

                            img.setPixel(x, y, sf::Color(red, green, blue));
                        }
                    }
                    w0 += w0_dx;
                    w1 += w1_dx;
                    w2 += w2_dx;
                }
                w0 -= w0_dx;
                w1 -= w1_dx;
                w2 -= w2_dx;
            }

            //auto x_tmp = (k & 1) ? x0 : x1;
            //glm::i16vec2 pixelSample((x_tmp << 6) + (1 << 5), ((y + 1) << 6) + (1 << 5));
            //int32_t w0_ref = ((int64_t)edgeFunction32(v1Raster, v2Raster, pixelSample) << 24) / area;
            //int32_t w1_ref = ((int64_t)edgeFunction32(v2Raster, v0Raster, pixelSample) << 24) / area;
            //int32_t w2_ref = ((int64_t)edgeFunction32(v0Raster, v1Raster, pixelSample) << 24) / area;

            w0 -= w0_dy;
            w1 -= w1_dy;
            w2 -= w2_dy;

        }
    }

    delete[] depthBuffer;

    //img.saveToFile("tmp.png");
}
