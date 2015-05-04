#ifndef HAL1_RINGBUFFER_H
#define HAL1_RINGBUFFER_H

static uint32_t const CONTROL_BAD_NAK_ID = (uint32_t) (-0x1);
static uint32_t const CONTROL_NAK_BUFFER_OVERFLOW = (uint32_t) (-0x2);
static uint32_t const CONTROL_MAX_HIGH_WATERMARK_ID = (uint32_t) (-0x3);
static uint32_t const MAX_HIGH_WATERMARK_VALUE = (uint32_t) (-0x100);

#endif //HAL1_RINGBUFFER_H
