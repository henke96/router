static void _x25519_testIterations(int32_t iterations, const void *k, const void *u, const void *expected) {
    uint8_t x[4][32];
    hc_MEMCPY(&x[0], u, 32);
    hc_MEMCPY(&x[1], k, 32);
    int32_t i = 0;
    for (; i < iterations; ++i) {
        x25519(&x[(i + 2) & 3][0], &x[(i + 1) & 3][0], &x[i & 3][0]);
    }
    CHECK(hc_MEMCMP(&x[(i + 1) & 3], expected, 32), RES == 0);
}

static void _x25519_testBasepointMult(int32_t iterations, const void *scalar, const void *expected) {
    uint8_t temp[64];
    hc_MEMCPY(&temp[0], scalar, 32);

    for (int32_t i = 0; i < iterations; ++i) {
        int32_t bit = i & 1;
        int32_t notBit = bit ^ 1;
        x25519(&temp[notBit << 5], &temp[bit << 5], &x25519_ecdhBasepoint[0]);
    }
    CHECK(hc_MEMCMP(&temp[(iterations & 1) << 5], expected, 32), RES == 0);

    // Same thing with ed25519.
    hc_MEMCPY(&temp[0], scalar, 32);
    for (int32_t i = 0; i < iterations; ++i) {
        int32_t bit = i & 1;
        int32_t notBit = bit ^ 1;
        ed25519_x25519Basepoint(&temp[notBit << 5], &temp[bit << 5]);
    }
    CHECK(hc_MEMCMP(&temp[(iterations & 1) << 5], expected, 32), RES == 0);
}

static void _x25519_testEcdh(const void *alicePriv, const void *bobPriv, const void *expectedShared) {
    uint8_t alicePub[32];
    uint8_t bobPub[32];
    x25519(&alicePub[0], alicePriv, &x25519_ecdhBasepoint[0]);
    x25519(&bobPub[0], bobPriv, &x25519_ecdhBasepoint[0]);

    uint8_t aliceShared[32];
    uint8_t bobShared[32];
    x25519(&aliceShared[0], alicePriv, &bobPub[0]);
    x25519(&bobShared[0], bobPriv, &alicePub[0]);

    CHECK(hc_MEMCMP(&aliceShared[0], expectedShared, 32), RES == 0);
    CHECK(hc_MEMCMP(&bobShared[0], expectedShared, 32), RES == 0);

    // Test generating the public keys with ed25519.
    uint8_t alicePubEd[32];
    uint8_t bobPubEd[32];
    ed25519_x25519Basepoint(&alicePubEd[0], alicePriv);
    ed25519_x25519Basepoint(&bobPubEd[0], bobPriv);
    CHECK(hc_MEMCMP(&alicePubEd[0], &alicePub[0], 32), RES == 0);
    CHECK(hc_MEMCMP(&bobPubEd[0], &bobPub[0], 32), RES == 0);
}

static void x25519_tests(void) {
    // Test vectors from RFC 7748.
    _x25519_testEcdh(
        &((uint8_t[]) { 0x77, 0x07, 0x6d, 0x0a, 0x73, 0x18, 0xa5, 0x7d, 0x3c, 0x16, 0xc1, 0x72, 0x51, 0xb2, 0x66, 0x45, 0xdf, 0x4c, 0x2f, 0x87, 0xeb, 0xc0, 0x99, 0x2a, 0xb1, 0x77, 0xfb, 0xa5, 0x1d, 0xb9, 0x2c, 0x2a })[0],
        &((uint8_t[]) { 0x5d, 0xab, 0x08, 0x7e, 0x62, 0x4a, 0x8a, 0x4b, 0x79, 0xe1, 0x7f, 0x8b, 0x83, 0x80, 0x0e, 0xe6, 0x6f, 0x3b, 0xb1, 0x29, 0x26, 0x18, 0xb6, 0xfd, 0x1c, 0x2f, 0x8b, 0x27, 0xff, 0x88, 0xe0, 0xeb })[0],
        &((uint8_t[]) { 0x4a, 0x5d, 0x9d, 0x5b, 0xa4, 0xce, 0x2d, 0xe1, 0x72, 0x8e, 0x3b, 0xf4, 0x80, 0x35, 0x0f, 0x25, 0xe0, 0x7e, 0x21, 0xc9, 0x47, 0xd1, 0x9e, 0x33, 0x76, 0xf0, 0x9b, 0x3c, 0x1e, 0x16, 0x17, 0x42 })[0]
    );
    _x25519_testIterations(
        1,
        &((uint8_t[]) { 0xa5, 0x46, 0xe3, 0x6b, 0xf0, 0x52, 0x7c, 0x9d, 0x3b, 0x16, 0x15, 0x4b, 0x82, 0x46, 0x5e, 0xdd, 0x62, 0x14, 0x4c, 0x0a, 0xc1, 0xfc, 0x5a, 0x18, 0x50, 0x6a, 0x22, 0x44, 0xba, 0x44, 0x9a, 0xc4 })[0],
        &((uint8_t[]) { 0xe6, 0xdb, 0x68, 0x67, 0x58, 0x30, 0x30, 0xdb, 0x35, 0x94, 0xc1, 0xa4, 0x24, 0xb1, 0x5f, 0x7c, 0x72, 0x66, 0x24, 0xec, 0x26, 0xb3, 0x35, 0x3b, 0x10, 0xa9, 0x03, 0xa6, 0xd0, 0xab, 0x1c, 0x4c })[0],
        &((uint8_t[]) { 0xc3, 0xda, 0x55, 0x37, 0x9d, 0xe9, 0xc6, 0x90, 0x8e, 0x94, 0xea, 0x4d, 0xf2, 0x8d, 0x08, 0x4f, 0x32, 0xec, 0xcf, 0x03, 0x49, 0x1c, 0x71, 0xf7, 0x54, 0xb4, 0x07, 0x55, 0x77, 0xa2, 0x85, 0x52 })[0]
    );
    _x25519_testIterations(
        1,
        &((uint8_t[]) { 0x4b, 0x66, 0xe9, 0xd4, 0xd1, 0xb4, 0x67, 0x3c, 0x5a, 0xd2, 0x26, 0x91, 0x95, 0x7d, 0x6a, 0xf5, 0xc1, 0x1b, 0x64, 0x21, 0xe0, 0xea, 0x01, 0xd4, 0x2c, 0xa4, 0x16, 0x9e, 0x79, 0x18, 0xba, 0x0d })[0],
        &((uint8_t[]) { 0xe5, 0x21, 0x0f, 0x12, 0x78, 0x68, 0x11, 0xd3, 0xf4, 0xb7, 0x95, 0x9d, 0x05, 0x38, 0xae, 0x2c, 0x31, 0xdb, 0xe7, 0x10, 0x6f, 0xc0, 0x3c, 0x3e, 0xfc, 0x4c, 0xd5, 0x49, 0xc7, 0x15, 0xa4, 0x93 })[0],
        &((uint8_t[]) { 0x95, 0xcb, 0xde, 0x94, 0x76, 0xe8, 0x90, 0x7d, 0x7a, 0xad, 0xe4, 0x5c, 0xb4, 0xb8, 0x73, 0xf8, 0x8b, 0x59, 0x5a, 0x68, 0x79, 0x9f, 0xa1, 0x52, 0xe6, 0xf8, 0xf7, 0x64, 0x7a, 0xac, 0x79, 0x57 })[0]
    );
    if (tests_level >= 1) _x25519_testIterations(
        1000,
        &x25519_ecdhBasepoint[0],
        &x25519_ecdhBasepoint[0],
        &((uint8_t[]) { 0x68, 0x4c, 0xf5, 0x9b, 0xa8, 0x33, 0x09, 0x55, 0x28, 0x00, 0xef, 0x56, 0x6f, 0x2f, 0x4d, 0x3c, 0x1c, 0x38, 0x87, 0xc4, 0x93, 0x60, 0xe3, 0x87, 0x5f, 0x2e, 0xb9, 0x4d, 0x99, 0x53, 0x2c, 0x51 })[0]
    );
    if (tests_level >= 2) _x25519_testIterations(
        1000000,
        &x25519_ecdhBasepoint[0],
        &x25519_ecdhBasepoint[0],
        &((uint8_t[]) { 0x7c, 0x39, 0x11, 0xe0, 0xab, 0x25, 0x86, 0xfd, 0x86, 0x44, 0x97, 0x29, 0x7e, 0x57, 0x5e, 0x6f, 0x3b, 0xc6, 0x01, 0xc0, 0x88, 0x3c, 0x30, 0xdf, 0x5f, 0x4d, 0xd2, 0xd2, 0x4f, 0x66, 0x54, 0x24 })[0]
    );
    // Test vector from ed25519-donna.
    if (tests_level >= 1) _x25519_testBasepointMult(
        1024,
        &((uint8_t[32]) { 0xff })[0],
        &((uint8_t[32]) { 0xac, 0xce, 0x24, 0xb1, 0xd4, 0xa2, 0x36, 0x21, 0x15, 0xe2, 0x3e, 0x84, 0x3c, 0x23, 0x2b, 0x5f, 0x95, 0x6c, 0xc0, 0x7b, 0x95, 0x82, 0xd7, 0x93, 0xd5, 0x19, 0xb6, 0xf1, 0xfb, 0x96, 0xd6, 0x04 })[0]
    );
}
