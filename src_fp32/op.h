#ifndef _OP_H_
#define _OP_H_

#include <cstddef>
#include <cstdint>

namespace mylenet
{ // to avoid INPUT macro mixing with windows.h
    typedef uint8_t INPUT;
    typedef float WEIGHT;
    typedef float BIAS;
    typedef float DATA;
}

template <typename I, typename D, size_t H, size_t W>
void preprocess(const I (&input)[H][W], D (&output)[1][H][W]);

template <typename D, typename W, typename B, size_t KH, size_t KW>
D conv_kernal(const D (&input)[KH][KW],
              const D (&kernal)[KH][KW]);

template <typename D, typename W, typename B,
          size_t IC, size_t OC,
          size_t IH, size_t IW,
          size_t KH, size_t KW,
          size_t OH, size_t OW>
void conv_relu(
    const D (&input)[IC][IH][IW],
    const W (&weights)[OC][IC][KH][KW],
    const B bias[OC],
    D (&output)[OC][OH][OW]);

template <typename D, typename W, typename B, size_t IH, size_t IW, size_t KH, size_t KW, size_t OH, size_t OW>
void _conv_relu(
    const D (&input)[IH][IW],
    const W (&weights)[KH][KW],
    D (&output)[OH][OW]);

template <typename D, size_t C, size_t IH, size_t IW, size_t OH, size_t OW>
void maxpooling(const D (&input)[C][IH][IW],
                size_t pool_size,
                size_t stride,
                D (&output)[C][OH][OW]);

template <typename D, typename W, typename B, size_t IL, size_t OL>
void fc_relu(D (&input)[IL], const W (&weights)[OL][IL], const B (&bias)[OL], D (&output)[OL]);

template <typename D, typename W, typename B, size_t IL, size_t OL>
void fc(D (&input)[IL], const W (&weights)[OL][IL], const B (&bias)[OL], D (&output)[OL]);

template <typename D, size_t C, size_t H, size_t W, size_t L>
void flatten(D (&din)[C][H][W], D (&dout)[L]);

template <typename D, size_t L>
size_t max_idx(const D (&rst)[L]);

template <typename T>
T relu(T x);

template <typename I, typename D, size_t H, size_t W>
void preprocess(const I (&input)[H][W], D (&output)[1][H][W])
{
preprocess_0:
    for (size_t i = 0; i < H; i++)
    {
	preprocess_1:
        for (size_t j = 0; j < W; j++)
        {
            output[0][i][j] = input[i][j] / 255.;
        }
    }
}

template <typename D, typename W, typename B, size_t KH, size_t KW>
D conv_kernal(const D (&input)[KH][KW],
              const D (&kernal)[KH][KW])
{
    D tmp = 0;
    conv_kernal_0:
    for (size_t i = 0; i < KH; i++)
    {
    	conv_kernal_1:
        for (size_t j = 0; j < KW; j++)
        {
            tmp += input[i][j] * kernal[i][j];
        }
    }
    return tmp;
}

template <typename D, typename W, typename B,
          size_t IC, size_t OC,
          size_t IH, size_t IW,
          size_t KH, size_t KW,
          size_t OH, size_t OW>
void conv_relu(
    const D (&input)[IC][IH][IW],
    const W (&weights)[OC][IC][KH][KW],
    const B bias[OC],
    D (&output)[OC][OH][OW])
{
	static D _conv_out[OH][OW];
	static D _conv_acc[OH][OW];

conv_oc:
    for (size_t i = 0; i < OC; i++)
    {
    conv_initialzie_acc_0:
        for (size_t k = 0; k < OH; k++)
        {
        conv_initialzie_acc_1:
            for (size_t l = 0; l < OW; l++)
            {
                _conv_acc[k][l] = 0;
            }
        }
    conv_acc_0:
        for (size_t j = 0; j < IC; j++)
        {
            _conv_relu<D, W, B, IH, IW, KH, KW, OH, OW>(input[j], weights[i][j], _conv_out);
        conv_acc_1:
            for (size_t k = 0; k < OH; k++)
            {
            conv_acc_2:
                for (size_t l = 0; l < OW; l++)
                {
                    _conv_acc[k][l] += _conv_out[k][l];
                }
            }
        }
    conv_bias_0:
        for (size_t k = 0; k < OH; k++)
        {
        conv_bias_1:
            for (size_t l = 0; l < OW; l++)
            {
                output[i][k][l] = relu<D>(_conv_acc[k][l] + bias[i]);
            }
        }
    }
}

template <typename D, typename W, typename B, size_t IH, size_t IW, size_t KH, size_t KW, size_t OH, size_t OW>
void _conv_relu(
    const D (&input)[IH][IW],
    const W (&weights)[KH][KW],
    D (&output)[OH][OW])
{
//    D window_buffer[KH][KW] = {0};
    static D window_buffer[KH][KW] = {0};
#pragma HLS ARRAY_PARTITION variable=window_buffer dim=0 complete
//    D line_buffer[KH][IW] = {0};
    static D line_buffer[KH][IW] = {0};
#pragma HLS ARRAY_PARTITION variable=line_buffer dim=0 complete

conv_initialize_line_buffer_0:
    for (size_t i = 0; i < KH - 1; i++)
    {
    conv_initialize_line_buffer_1:
        for (size_t j = 0; j < IW; j++)
        {
            line_buffer[i + 1][j] = input[i][j];
        }
    }

conv_0:
    for (size_t i = KH - 1; i < IH; i++)
    {
    conv_1:
        for (size_t j = 0; j < IW; j++)
        {
        conv_shift_window_buffer_0:
            for (size_t k = 0; k < KH; k++)
            {
            conv_shift_line_buffer:
                if (k == KH - 1)
                {
                    line_buffer[k][j] = input[i][j];
                }
                else
                {
                    line_buffer[k][j] = line_buffer[k + 1][j];
                }
            conv_shift_window_buffer_1:
                for (size_t l = 0; l < KW - 1; l++)
                {
                    window_buffer[k][l] = window_buffer[k][l + 1];
                }
                window_buffer[k][KW - 1] = line_buffer[k][j];
            }
            if (j >= KW - 1)
            {
                output[i - KH + 1][j - KW + 1] = conv_kernal<D, W, B, KH, KW>(window_buffer, weights);
            }
        }
    }
}

template <typename D, size_t C, size_t IH, size_t IW, size_t OH, size_t OW>
void maxpooling(const D (&input)[C][IH][IW],
                size_t pool_size,
                size_t stride,
                D (&output)[C][OH][OW])
{
    D tmp, ele;
maxpooling_0:
    for (size_t c = 0; c < C; c++)
    {
    maxpooling_1:
        for (size_t i = 0; i < OH; i++)
        {
        maxpooling_2:
            for (size_t j = 0; j < OW; j++)
            {
                tmp = 0;
            maxpooling_3:
                for (size_t k = 0; k < pool_size; k++)
                {
                maxpooling_4:
                    for (size_t l = 0; l < pool_size; l++)
                    {
                        ele = input[c][i * stride + k][j * stride + l];
                        tmp = (ele > tmp) ? ele : tmp;
                    }
                }
                output[c][i][j] = tmp;
            }
        }
    }
}

template <typename D, typename W, typename B, size_t IL, size_t OL>
void fc_relu(D (&input)[IL], const W (&weights)[OL][IL], const B (&bias)[OL], D (&output)[OL])
{
//    static D out_tmp[OL] = {0};
	D out_tmp[OL] = {0};
fc_0:
	for (size_t nin = 0; nin < IL; nin++)
    {
    fc_1:
		for (size_t nout = 0; nout < OL; nout++)
		{
			out_tmp[nout] += weights[nout][nin] * input[nin];
		}
    }
	for (size_t nout = 0; nout < OL; nout++) {
		output[nout] = relu<D>(out_tmp[nout] + bias[nout]);
	}
}

template <typename D, typename W, typename B, size_t IL, size_t OL>
void fc(D (&input)[IL], const W (&weights)[OL][IL], const B (&bias)[OL], D (&output)[OL])
{
//    static D out_tmp[OL] = {0};
	D out_tmp[OL] = {0};
fc_0:
	for (size_t nin = 0; nin < IL; nin++)
    {
    fc_1:
		for (size_t nout = 0; nout < OL; nout++)
		{
			out_tmp[nout] += weights[nout][nin] * input[nin];
		}
    }
	for (size_t nout = 0; nout < OL; nout++) {
		output[nout] = out_tmp[nout] + bias[nout];
	}
}

template <typename D, size_t C, size_t H, size_t W, size_t L>
void flatten(D (&din)[C][H][W], D (&dout)[L])
{
    size_t l = 0;
    for (size_t c = 0; c < C; c++)
    {
        for (size_t h = 0; h < H; h++)
        {
            for (size_t w = 0; w < W; w++)
            {
                dout[l++] = din[c][h][w];
            }
        }
    }
}

template <typename D, size_t L>
size_t max_idx(const D (&rst)[L])
{
    D tmp = rst[0];
    size_t idx = 0;
max_idx:
    for (size_t i = 1; i < 10; i++)
    {
        if (rst[i] > tmp)
        {
            tmp = rst[i];
            idx = i;
        }
    }
    return idx;
}

template <typename T>
T relu(T x)
{
    if (x > 0)
        return x;
    else
        return 0;
}

#endif
