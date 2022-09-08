// TCode-Buffer-class v1.0,
// protocal by TempestMAx (https://www.patreon.com/tempestvr)
// implemented by Eve 04/06/2022
// usage of this class can be found at (https://github.com/Dreamer2345/Arduino_TCode_Parser)
// Please copy, share, learn, innovate, give attribution.
// Decodes T-code commands
// It can handle:
//   x linear channels (L0, L1, L2... L9)
//   x rotation channels (R0, R1, R2... L9)
//   x vibration channels (V0, V1, V2... V9)
//   x auxilliary channels (A0, A1, A2... A9)
// History:
//

#pragma once
#ifndef TCODE_BUFFER_H
#define TCODE_BUFFER_H

template <unsigned TCODE_BUFFER_LENGTH = 127>
class TCodeBuffer
{
private:
    unsigned Front = 0;
    unsigned Back = 0;
    char buffer[TCODE_BUFFER_LENGTH];

public:
    bool isFull();
    bool isEmpty();
    void clear();

    unsigned count();

    bool push(char obj);
    char pop(bool &success);
    char peek(bool &success);
    char pop();
    char peek();
};

template <unsigned TCODE_BUFFER_LENGTH>
bool TCodeBuffer<TCODE_BUFFER_LENGTH>::isEmpty()
{
    return Front == Back;
}

template <unsigned TCODE_BUFFER_LENGTH>
void TCodeBuffer<TCODE_BUFFER_LENGTH>::clear()
{
    Front = Back;
}

template <unsigned TCODE_BUFFER_LENGTH>
bool TCodeBuffer<TCODE_BUFFER_LENGTH>::isFull()
{
    return (Front == ((Back + 1) % TCODE_BUFFER_LENGTH));
}

template <unsigned TCODE_BUFFER_LENGTH>
unsigned TCodeBuffer<TCODE_BUFFER_LENGTH>::count()
{
    if (Back >= Front)
        return Back - Front;

    return TCODE_BUFFER_LENGTH - (Front - Back);
}

template <unsigned TCODE_BUFFER_LENGTH>
bool TCodeBuffer<TCODE_BUFFER_LENGTH>::push(char obj)
{
    if (isFull())
    {
        return false;
    }
    buffer[Back] = obj;
    Back = (Back + 1) % TCODE_BUFFER_LENGTH;
    return true;
}

template <unsigned TCODE_BUFFER_LENGTH>
char TCodeBuffer<TCODE_BUFFER_LENGTH>::pop(bool &success)
{
    if (isEmpty())
    {
        success = false;
        return 0;
    }

    success = true;
    char headItem = buffer[Front];
    Front = (Front + 1) % TCODE_BUFFER_LENGTH;
    return headItem;
}

template <unsigned TCODE_BUFFER_LENGTH>
char TCodeBuffer<TCODE_BUFFER_LENGTH>::peek(bool &success)
{
    if (isEmpty())
    {
        success = false;
        return 0;
    }

    success = true;
    char headItem = buffer[Front];
    return headItem;
}

template <unsigned TCODE_BUFFER_LENGTH>
char TCodeBuffer<TCODE_BUFFER_LENGTH>::pop()
{
    if (isEmpty())
    {
        return 0;
    }

    char headItem = buffer[Front];
    Front = (Front + 1) % TCODE_BUFFER_LENGTH;
    return headItem;
}

template <unsigned TCODE_BUFFER_LENGTH>
char TCodeBuffer<TCODE_BUFFER_LENGTH>::peek()
{
    if (isEmpty())
    {
        return 0;
    }

    char headItem = buffer[Front];
    return headItem;
}

#endif
