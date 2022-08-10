#include <iostream>
#include <map>
#include <unordered_map>
#include <algorithm>

//Класс малый аллокатор
class SmallAllocator {

private:
    char Memory[1048576];
    std::unordered_multimap<unsigned int, unsigned int> freeMemoryParts;
    std::unordered_map<char *, unsigned int> busyMemoryParts;

public:
    SmallAllocator()
    {
        //Вся память свободна
        freeMemoryParts.insert(std::make_pair(sizeof(Memory) / sizeof(char), 0));
    };

    //Выделяет блок памяти необходимого размера
    void *Alloc(unsigned int Size) {
        if (Size == 0)
            return NULL;

        //Ищем в памяти свободный блок памяти равный необходимому размеру
        auto freeMemoryPart = freeMemoryParts.find(Size);
        if (freeMemoryPart == freeMemoryParts.end())
        {
            //Ищем в памяти свободный блок памяти больше необходимого размера
            freeMemoryPart = std::find_if(freeMemoryParts.begin(),
                                          freeMemoryParts.end(),
                                          [Size]
                                          (const std::pair<unsigned int, unsigned int> &freeMemoryPart) -> bool { return freeMemoryPart.first > Size;});
        }
        if (freeMemoryPart == freeMemoryParts.end())
            return NULL;

        //Запоминаем данные о свободном блоке памяти, с которым будем работать
        std::pair<unsigned int, unsigned int> memoryPart = (*freeMemoryPart);

        //Удаляем информацию о свободном блоке памяти
        freeMemoryParts.erase(freeMemoryPart);

        //Если был найден свободный блок памяти больше необходимого размера
        if (memoryPart.first > Size)
        {
            //Если следующий свободный блок памяти по индексу в рамках памяти
            if (memoryPart.second < sizeof(Memory) / sizeof(char) - 1)
            {
                unsigned int nextFreeMemoryIndex = memoryPart.second + 1;
                auto nextFreeMemoryPart = std::find_if(freeMemoryParts.begin(),
                                                       freeMemoryParts.end(),
                                                       [nextFreeMemoryIndex]
                                                       (const std::pair<unsigned int, unsigned int> &freeMemoryPart) -> bool { return freeMemoryPart.second == nextFreeMemoryIndex;});
                //Добавляем информацию об остатке свободной памяти
                std::pair<unsigned int, unsigned int> newFreeMemoryPart = std::make_pair(memoryPart.first - Size, memoryPart.second + Size);
                if (nextFreeMemoryPart != freeMemoryParts.end())
                {
                    newFreeMemoryPart.first += nextFreeMemoryPart->first;
                    freeMemoryParts.erase(nextFreeMemoryPart);
                }

                freeMemoryParts.insert(newFreeMemoryPart);
            }
        }

        busyMemoryParts.insert(std::make_pair(&Memory[memoryPart.second], Size));
        return &Memory[memoryPart.second];
    };

    //Перевыделяет блок памяти необходимого размера
    void *ReAlloc(void *Pointer, unsigned int Size) {
        char *busyMemoryPointer = (char *) Pointer;

        //Ищем в памяти занятый блок памяти
        auto busyMemoryPart = busyMemoryParts.find(busyMemoryPointer);
        if (busyMemoryPart == busyMemoryParts.end())
            return NULL;

        //Если памяти нужно столько же: выход
        if (Size == busyMemoryPart->second)
            return Pointer;

        //Если памяти не нужно: освобождаем блок памяти и выход
        if (Size == 0)
        {
            Free(Pointer);
            return NULL;
        }

        unsigned int memoryIndex = busyMemoryPointer - Memory;

        //Если памяти нужно меньше
        if (Size < busyMemoryPart->second)
        {
            //Создаем новый блок свободной памяти
            unsigned int newFreeMemoryPartIndex = memoryIndex + Size;
            unsigned int nextFreeMemoryIndex = memoryIndex + busyMemoryPart->second;
            auto nextFreeMemoryPart = std::find_if(freeMemoryParts.begin(),
                                                   freeMemoryParts.end(),
                                                   [nextFreeMemoryIndex]
                                                   (const std::pair<unsigned int, unsigned int> &freeMemoryPart) -> bool { return freeMemoryPart.second == nextFreeMemoryIndex;});
            std::pair<unsigned int, unsigned int> newFreeMemoryPart = std::make_pair(busyMemoryPart->second - Size, newFreeMemoryPartIndex);
            if (nextFreeMemoryPart != freeMemoryParts.end())
            {
                newFreeMemoryPart.first += nextFreeMemoryPart->first;
                freeMemoryParts.erase(nextFreeMemoryPart);
            }
            freeMemoryParts.insert(newFreeMemoryPart);

            //Обновляем информацию о занятом блоке памяти
            busyMemoryPart->second = Size;
            return Pointer;
        }

        //Если памяти нужно больше
        //Проверяем наличие свободного блока памяти недостающего размера после текущего блока памяти
        unsigned int nextFreeMemoryIndex = memoryIndex + busyMemoryPart->second;
        unsigned int additionalMemorySize = Size - busyMemoryPart->second;
        auto nextFreeMemoryPart = std::find_if(freeMemoryParts.begin(),
                                               freeMemoryParts.end(),
                                               [nextFreeMemoryIndex, additionalMemorySize]
                                               (const std::pair<unsigned int, unsigned int> &freeMemoryPart) -> bool { return (freeMemoryPart.second == nextFreeMemoryIndex
                                                                                                                               && freeMemoryPart.first >= additionalMemorySize);});
        //Если найден свободный блок памяти недостающего размера, сразу после текущего блока памяти
        if (nextFreeMemoryPart != freeMemoryParts.end())
        {
            //Обновляем информацию о занятом блоке памяти
            busyMemoryPart->second = Size;

            //Обновляем информацию о свободном блоке памяти
            if (nextFreeMemoryPart->first > additionalMemorySize)
                freeMemoryParts.insert(std::make_pair(nextFreeMemoryPart->first - additionalMemorySize, nextFreeMemoryIndex + additionalMemorySize));
            freeMemoryParts.erase(nextFreeMemoryPart);

            return Pointer;
        }

        //Если не найден свободный блок памяти недостающего размера, сразу после текущего блока памяти
        //Выделяем блок памяти необходимого размера
        char *newBusyMemoryPointer = (char *) Alloc(Size);

        //Копируем содержимое прошлого занятого блока памяти
        for (int i = 0; i < busyMemoryPart->second; ++i)
            *(newBusyMemoryPointer + i) = *(busyMemoryPointer + i);

        //Освобождаем прошлый блок памяти
        Free(Pointer);

        return newBusyMemoryPointer;
    };

    //Освобождает блок памяти
    void Free(void *Pointer) {
        char *busyMemoryPointer = (char *) Pointer;

        //Ищем в памяти занятый блок памяти
        auto busyMemoryPart = busyMemoryParts.find(busyMemoryPointer);
        if (busyMemoryPart == busyMemoryParts.end())
            return;

        unsigned int memoryIndex = busyMemoryPointer - Memory;

        //Создаем новый свободный блок памяти
        std::pair<unsigned int, unsigned int> newFreeMemoryPart = std::make_pair(busyMemoryPart->second, 0);

        //Удаляем информацию о занятом блоке памяти
        busyMemoryParts.erase(busyMemoryPointer);

        //Пытаемся объединить текущий блок памяти с свободным блоком сверху
        unsigned int nearFreeMemoryPartIndex = 0;
        auto nearFreeMemoryPart = freeMemoryParts.end();
        if (memoryIndex > 0)
        {
            nearFreeMemoryPartIndex = memoryIndex - 1;
            nearFreeMemoryPart = std::find_if(freeMemoryParts.begin(),
                                              freeMemoryParts.end(),
                                              [nearFreeMemoryPartIndex]
                                              (const std::pair<unsigned int, unsigned int> &freeMemoryPart) -> bool { return freeMemoryPart.second == nearFreeMemoryPartIndex;});
        }

        //Пытаемся объединить текущий блок памяти с свободным блоком снизу
        if (nearFreeMemoryPart == freeMemoryParts.end()
                && memoryIndex < sizeof(Memory) / sizeof(char) - 1)
        {
            nearFreeMemoryPartIndex = memoryIndex + newFreeMemoryPart.first;
            nearFreeMemoryPart = std::find_if(freeMemoryParts.begin(),
                                              freeMemoryParts.end(),
                                              [nearFreeMemoryPartIndex]
                                              (const std::pair<unsigned int, unsigned int> &freeMemoryPart) -> bool { return freeMemoryPart.second == nearFreeMemoryPartIndex;});

        }

        //Корректируем информацию о новом свободном блоке памяти (из-за объединения свободных блоков)
        if (nearFreeMemoryPart != freeMemoryParts.end())
        {
            newFreeMemoryPart.first += nearFreeMemoryPart->first;
            if (nearFreeMemoryPartIndex < memoryIndex)
                newFreeMemoryPart.second = nearFreeMemoryPart->second;
            else
                newFreeMemoryPart.second = memoryIndex;

            freeMemoryParts.erase(nearFreeMemoryPart);
        }

        //Добавляем информацию о новом блоке свободной памяти
        freeMemoryParts.insert(newFreeMemoryPart);
    };
};

int main(int argc, char *argv[])
{
    SmallAllocator A1;
    int * A1_P1 = (int *) A1.Alloc(sizeof(int));
    A1_P1 = (int *) A1.ReAlloc(A1_P1, 2 * sizeof(int));
    A1.Free(A1_P1);
    SmallAllocator A2;
    int * A2_P1 = (int *) A2.Alloc(10 * sizeof(int));
    for(unsigned int i = 0; i < 10; i++) A2_P1[i] = i;
    for(unsigned int i = 0; i < 10; i++) if(A2_P1[i] != i) std::cout << "ERROR 1" << std::endl;
    int * A2_P2 = (int *) A2.Alloc(10 * sizeof(int));
    for(unsigned int i = 0; i < 10; i++) A2_P2[i] = -1;
    for(unsigned int i = 0; i < 10; i++) if(A2_P1[i] != i) std::cout << "ERROR 2" << std::endl;
    for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 3" << std::endl;
    A2_P1 = (int *) A2.ReAlloc(A2_P1, 20 * sizeof(int));
    for(unsigned int i = 10; i < 20; i++) A2_P1[i] = i;
    for(unsigned int i = 0; i < 20; i++) if(A2_P1[i] != i) std::cout << "ERROR 4" << std::endl;
    for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 5" << std::endl;
    A2_P1 = (int *) A2.ReAlloc(A2_P1, 5 * sizeof(int));
    for(unsigned int i = 0; i < 5; i++) if(A2_P1[i] != i) std::cout << "ERROR 6" << std::endl;
    for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 7" << std::endl;
    A2.Free(A2_P1);
    A2.Free(A2_P2);

    return 0;
}
