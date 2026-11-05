#include <iostream>
#include <stack>

void ordenarPila(std::stack<int>& s) {
    std::stack<int> aux;
    while (!s.empty()) {
        int temp = s.top();
        s.pop();
        while (!aux.empty() && aux.top() > temp) {
            s.push(aux.top());
            aux.pop();
        }
        aux.push(temp);
    }
    s = aux;
}

int main() {
    std::stack<int> s;
    s.push(3);
    s.push(1);
    s.push(4);
    s.push(2);
    s.push(5);

    ordenarPila(s);

    while (!s.empty()) {
        std::cout << s.top() << " ";
        s.pop();
    }
    std::cout << std::endl;
    return 0;
}

#include <iostream>
#include <string>
#include <sstream>

int main() {
    std::string expresion = "6+8-4";
    std::stringstream ss(expresion);
    int resultado = 0;
    int num;
    char op = '+';

    while (ss >> num) {
        if (op == '+') {
            resultado += num;
        } else if (op == '-') {
            resultado -= num;
        }
        ss >> op;
    }

    std::cout << "Resultado: " << resultado << std::endl;
    return 0;
}

