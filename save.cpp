#include "game.hpp"

static constexpr uint8_t SAVE_VERSION = 1;

#ifdef ARDUINO
#include <EEPROM.h>
static inline void eeprom_update(int addr, uint8_t data)
{
    EEPROM.update(addr, data);
}
static inline uint8_t eeprom_read(int addr)
{
    return EEPROM.read(addr);
}
#else
static inline void eeprom_update(int addr, uint8_t data)
{
    (void)addr;
    (void)data;
}
static inline uint8_t eeprom_read(int addr)
{
    return 0;
}
#endif

static constexpr int EEPROM_END = 1024;
static constexpr int EEPROM_START = EEPROM_END - SAVE_FILE_BYTES - 3;
static constexpr int CHECKSUM_ADDR = EEPROM_END - 2;
static constexpr int VERSION_ADDR = EEPROM_END - 3;

static void set_save_checksum(uint16_t x)
{
    eeprom_update(CHECKSUM_ADDR + 0, *((uint8_t*)&x + 0));
    eeprom_update(CHECKSUM_ADDR + 1, *((uint8_t*)&x + 1));
}

static uint16_t get_save_checksum()
{
    uint16_t x;
    *((uint8_t*)&x + 0) = eeprom_read(CHECKSUM_ADDR);
    *((uint8_t*)&x + 1) = eeprom_read(CHECKSUM_ADDR + 1);
    return x;
}

static uint16_t compute_checksum()
{
    // CRC16
    uint8_t x;
    uint16_t crc = 0xffff;
    for(int i = EEPROM_START; i < EEPROM_END - 2; ++i)
    {
        x = (crc >> 8) ^ eeprom_read(i);
        x ^= x >> 4;
        crc = (crc << 8) ^
            (uint16_t(x) << 12) ^
            (uint16_t(x) << 5) ^
            (uint16_t(x) << 0);
    }
    return crc;
}

static bool save_is_valid()
{
    return eeprom_read(VERSION_ADDR) == SAVE_VERSION;
}

bool save_exists()
{
    return save_is_valid() && compute_checksum() == get_save_checksum();
}

void destroy_save()
{
    eeprom_update(VERSION_ADDR, ~SAVE_VERSION);
}

void save()
{
    for(uint16_t i = 0; i < SAVE_FILE_BYTES; ++i)
        eeprom_update(EEPROM_START + i, ((uint8_t*)&globals_.saved)[i]);
    eeprom_update(VERSION_ADDR, SAVE_VERSION);
    set_save_checksum(compute_checksum());
}

void load()
{
    for(uint16_t i = 0; i < SAVE_FILE_BYTES; ++i)
        ((uint8_t*)&globals_.saved)[i] = eeprom_read(EEPROM_START + i);
}
