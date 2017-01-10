#include <stdlib.h>
#include "font.h"
#include "font.data"

#if INTERFACE

typedef struct {
    int Width;
    int Height;
    int Chars;
    uint8_t *Data;
} Font;

#endif

Font Terminus12Bold = {
    .Width = 6,
    .Height = 12,
    .Chars = 128,
    .Data = Terminus12BoldData,
};

Font Terminus12Medium = {
    .Width = 6,
    .Height = 12,
    .Chars = 128,
    .Data = Terminus12MediumData,
};

Font Terminus14Bold = {
    .Width = 8,
    .Height = 14,
    .Chars = 128,
    .Data = Terminus14BoldData,
};

Font Terminus14Medium = {
    .Width = 8,
    .Height = 14,
    .Chars = 128,
    .Data = Terminus14MediumData,
};

Font Terminus16Bold = {
    .Width = 8,
    .Height = 16,
    .Chars = 128,
    .Data = Terminus16BoldData,
};

Font Terminus16Medium = {
    .Width = 8,
    .Height = 16,
    .Chars = 128,
    .Data = Terminus16MediumData,
};

Font Terminus18Bold = {
    .Width = 10,
    .Height = 18,
    .Chars = 128,
    .Data = Terminus18BoldData,
};

Font Terminus18Medium = {
    .Width = 10,
    .Height = 18,
    .Chars = 128,
    .Data = Terminus18MediumData,
};

Font Terminus20Bold = {
    .Width = 10,
    .Height = 20,
    .Chars = 128,
    .Data = Terminus20BoldData,
};

Font Terminus20Medium = {
    .Width = 10,
    .Height = 20,
    .Chars = 128,
    .Data = Terminus20MediumData,
};

Font Terminus22Bold = {
    .Width = 11,
    .Height = 22,
    .Chars = 128,
    .Data = Terminus22BoldData,
};

Font Terminus22Medium = {
    .Width = 11,
    .Height = 22,
    .Chars = 128,
    .Data = Terminus22MediumData,
};

Font Terminus24Bold = {
    .Width = 12,
    .Height = 24,
    .Chars = 128,
    .Data = Terminus24BoldData,
};

Font Terminus24Medium = {
    .Width = 12,
    .Height = 24,
    .Chars = 128,
    .Data = Terminus24MediumData,
};

Font Terminus28Bold = {
    .Width = 14,
    .Height = 28,
    .Chars = 128,
    .Data = Terminus28BoldData,
};

Font Terminus28Medium = {
    .Width = 14,
    .Height = 28,
    .Chars = 128,
    .Data = Terminus28MediumData,
};

Font Terminus32Bold = {
    .Width = 16,
    .Height = 32,
    .Chars = 128,
    .Data = Terminus32BoldData,
};

Font Terminus32Medium = {
    .Width = 16,
    .Height = 32,
    .Chars = 128,
    .Data = Terminus32MediumData,
};
