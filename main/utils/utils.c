int map_value(int value, int from, int to)
{
    int scaledValue = (value * to) / from;
    // Проверка на выход из диапазона от 0 до 255
    if (scaledValue < 0) {
        scaledValue = 0;
    } else if (scaledValue > to) {
        scaledValue = to;
    }
    return scaledValue;
}