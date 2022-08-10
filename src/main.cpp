#include <iostream>
#include <regex>
#include <map>

//Возвращает производную от многочлена
std::string derivative(std::string polynomial) {
    std::regex xCoefficientAndPowerRegex("(?:([+-])?(?:(\\d+)\\*)?)?x(?:\\^(\\d+))?");
    std::regex xPartRegex("((?:[+-]?(?:\\d+\\*)?)?x(?:\\^\\d+)?)");
    polynomial.erase(remove_if(polynomial.begin(), polynomial.end(), isspace), polynomial.end());

    //Разбираем многочлен
    std::map<unsigned long long, long long> derivedPolynomialData;
    for (std::smatch match; std::regex_search(polynomial, match, xPartRegex); polynomial = match.suffix())
    {
        //Получили часть многочлена
        std::string polynomialPart = match[1];

        //Получаем разбор части многочлена на степень и коэффициент
        std::smatch coefficientAndPowerMatch;
        std::regex_search(polynomialPart, coefficientAndPowerMatch, xCoefficientAndPowerRegex);

        //Получаем степень
        std::string powerStr = coefficientAndPowerMatch[3];
        if (powerStr.empty())
            powerStr = "1";
        unsigned long long nextPower = std::stoull(powerStr) - 1;

        //Получаем коэффициент
        std::string coefficientStr = coefficientAndPowerMatch[1];
        coefficientStr += coefficientAndPowerMatch[2];
        if (coefficientStr.empty()
                || coefficientStr == "-"
                || coefficientStr == "+")
            coefficientStr += "1";
        long long nextCoefficient = std::stoll(coefficientStr) * (nextPower + 1);

        //Запоминаем степень и коэффициент
        auto searchPowerResult = derivedPolynomialData.find(nextPower);
        if (searchPowerResult == derivedPolynomialData.end())
            derivedPolynomialData[nextPower] = nextCoefficient;
        else
            derivedPolynomialData[nextPower] += nextCoefficient;
    }

    //Формируем строку для вывода
    std::string derivedPolynomial = "";
    for (auto i = derivedPolynomialData.crbegin(); i != derivedPolynomialData.crend(); ++i)
    {
        //Если коэффициент стал 0: пропускаем член
        if ((*i).second == 0)
            continue;

        //Если не первый член и коэффициент > 0: добавляем знак +
        if (i != derivedPolynomialData.crbegin()
                && (*i).second > 0)
            derivedPolynomial += "+";

        //Если коэффициент по модулю не равен 1: добавляем коэффициент
        if (abs((*i).second) != 1
                || (*i).first == 0)
            derivedPolynomial += std::to_string((*i).second);

        //Если степень больше 0: добавляем знак x
        if ((*i).first > 0)
            derivedPolynomial += "*x";

        //Если степень больше единицы: добавляем выражение степени
        if ((*i).first > 1)
            derivedPolynomial += "^" + std::to_string((*i).first);
    }

    return derivedPolynomial;
}

int main(int argc, char *argv[])
{
    //Пример
    std::regex xCoefficientAndPowerRegex("(?:([+-])?(?:(\\d+)\\*)?)?x(?:\\^(\\d+))?");
    std::regex xPartRegex("((?:[+-]?(?:\\d+\\*)?)?x(?:\\^\\d+)?)");
    std::vector<std::string> polynomials = std::vector<std::string>({
                                                                        "x^2+x",
                                                                        "2*x^100+100*x^2",
                                                                        "x^10000+x+1",
                                                                        "-x^2-x^3",
                                                                        "x+x+x+x+x+x+x+x+x+x"
                                                                    });
    std::vector<std::string> xParts;
    for (auto i = polynomials.begin(); i != polynomials.end(); ++i)
        std::cout << derivative(*i) << std::endl;

    return 0;
}
