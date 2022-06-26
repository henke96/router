static uint8_t base64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define base64_ENCODE_SIZE(INPUT_SIZE) (((INPUT_SIZE + 2) / 3) * 4)

static int64_t base64_encode(uint8_t *output, uint8_t *input, int64_t inputSize) {
    uint8_t *end = &input[inputSize];
    uint8_t *outputPos = &output[0];

    while (end - input >= 3) {
        outputPos[0] = base64_table[input[0] >> 2];
        outputPos[1] = base64_table[((input[0] & 0x3) << 4) | (input[1] >> 4)];
        outputPos[2] = base64_table[((input[1] & 0xF) << 2) | (input[2] >> 6)];
        outputPos[3] = base64_table[input[2] & 0x3F];

        input += 3;
        outputPos += 4;
    }

    int64_t remaining = (int64_t)(end - input);
    if (remaining > 0) {
        outputPos[0] = base64_table[input[0] >> 2];
        if (remaining == 1) {
            outputPos[1] = base64_table[(input[0] & 0x3) << 4];
            outputPos[2] = '=';
        } else { // remaining == 2
            outputPos[1] = base64_table[((input[0] & 0x3) << 4) | (input[1] >> 4)];
            outputPos[2] = base64_table[((input[1] & 0xF) << 2)];
        }
        outputPos[3] = '=';
        outputPos += 4;
    }
    return (int64_t)(outputPos - output);
}