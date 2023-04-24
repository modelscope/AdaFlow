#include "common.h"

gint find_key_strv(const gchar** strv, const gchar* key) {
    gint cursor = 0;

    while (strv[cursor] && key) {
        if (g_ascii_strcasecmp(strv[cursor], key) == 0) return cursor;
        cursor++;
    }

    return -1; /* Not Found */
}

void I420uv_resize(const gfloat* input, gint src_w, gint src_h, gfloat* output,
                   gint dst_w, gint dst_h) {
    gint xnn_size_input = src_h * src_w;
    gint xnn_size_output = dst_h * dst_w;

    gint xnn_size_inputuv = xnn_size_input + src_h * src_w / 4;
    gint xnn_size_outputuv = xnn_size_output + dst_h * dst_w / 4;

    float x_ratio, y_ratio;

    x_ratio = (float)(src_w - 1) / (float)dst_w;
    y_ratio = (float)(src_h - 1) / (float)dst_h;

    gint dstuv_w = dst_w / 2;
    gint dstuv_h = dst_h / 2;

    gint srcuv_w = src_w / 2;
    gint srcuv_h = src_h / 2;

    float x, y;
    float u, v;
    int col, row;
    float a, b, c, d;
    float val;

    int col_l, row_l;
    int col_h, row_h;

    for (int i = 0; i < dstuv_h; i++) {
        for (int j = 0; j < dstuv_w; j++) {
            x = x_ratio * j;
            y = y_ratio * i;
            row = (int)y;
            col = (int)x;

            u = x - col;
            v = y - row;

            col_l = MAX(col, 0);
            row_l = MAX(row, 0);

            col_h = MIN(col + 1, srcuv_w - 1);
            row_h = MIN(row + 1, srcuv_h - 1);

            a = input[xnn_size_input + row_l * srcuv_w + col_l];
            b = input[xnn_size_input + row_l * srcuv_w + col_h];
            c = input[xnn_size_input + row_h * srcuv_w + col_l];
            d = input[xnn_size_input + row_h * srcuv_w + col_h];
            val = (a * (1 - u) * (1 - v) + b * (1 - v) * u + c * (1 - u) * v +
                   d * u * v);
            val = CLAMP(val, 0, 255);
            output[xnn_size_output + i * dstuv_w + j] = val;

            a = input[xnn_size_inputuv + row_l * srcuv_w + col_l];
            b = input[xnn_size_inputuv + row_l * srcuv_w + col_h];
            c = input[xnn_size_inputuv + row_h * srcuv_w + col_l];
            d = input[xnn_size_inputuv + row_h * srcuv_w + col_h];
            val = (a * (1 - u) * (1 - v) + b * (1 - v) * u + c * (1 - u) * v +
                   d * u * v);
            val = CLAMP(val, 0, 255);
            output[xnn_size_outputuv + i * dstuv_w + j] = val;
        }
    }
}

void NVuv_resize(const gfloat* input, gint src_w, gint src_h, gfloat* output,
                 gint dst_w, gint dst_h) {
    gint xnn_size_input = src_h * src_w;
    gint xnn_size_output = dst_h * dst_w;

    float x_ratio, y_ratio;

    x_ratio = (float)(src_w - 1) / (float)dst_w;
    y_ratio = (float)(src_h - 1) / (float)dst_h;

    gint dstuv_w = dst_w / 2;
    gint dstuv_h = dst_h / 2;

    gint srcuv_w = src_w / 2;
    gint srcuv_h = src_h / 2;

    float x, y;
    float u, v;
    int col, row;
    float a, b, c, d;
    float val;

    int col_l, row_l;
    int col_h, row_h;

    for (int i = 0; i < dstuv_h; i++) {
        for (int j = 0; j < dstuv_w; j++) {
            x = x_ratio * j;
            y = y_ratio * i;
            row = (int)y;
            col = (int)x;

            u = x - col;
            v = y - row;

            col_l = MAX(col * 2, 0);
            row_l = MAX(row, 0);

            col_h = MIN((col + 1) * 2, src_w - 1);
            row_h = MIN(row + 1, srcuv_h - 1);

            a = input[xnn_size_input + row_l * src_w + col_l];
            b = input[xnn_size_input + row_l * src_w + col_h];
            c = input[xnn_size_input + row_h * src_w + col_l];
            d = input[xnn_size_input + row_h * src_w + col_h];
            val = (a * (1 - u) * (1 - v) + b * (1 - v) * u + c * (1 - u) * v +
                   d * u * v);
            val = CLAMP(val, 0, 255);
            output[xnn_size_output + i * dst_w + j * 2] = val;

            col_l = MAX(col * 2 + 1, 0);
            col_h = MIN((col + 1) * 2 + 1, src_w - 1);
            a = input[xnn_size_input + row_l * src_w + col_l];
            b = input[xnn_size_input + row_l * src_w + col_h];
            c = input[xnn_size_input + row_h * src_w + col_l];
            d = input[xnn_size_input + row_h * src_w + col_h];
            val = (a * (1 - u) * (1 - v) + b * (1 - v) * u + c * (1 - u) * v +
                   d * u * v);
            val = CLAMP(val, 0, 255);
            output[xnn_size_output + i * dst_w + j * 2 + 1] = val;
        }
    }
}