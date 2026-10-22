// crear un programa que permita revertir una lista doble enlazada en un solo recorrido

#include <iostream>
using namespace std;


struct Node {
    int data;
    Node* prev;
    Node* next;
    
    Node(int val) {
        data = val;
        prev = nullptr;
        next = nullptr;
    }
};

Node* reverseDoublyLinkedList(Node* head) {
    if (!head || !head->next) {
        return head;
    }
    
    Node* current = head;
    Node* temp = nullptr;
    
    while (current != nullptr) {
        temp = current->prev;
        current->prev = _____________;
        current->next = temp;
        
        head = current;
        current = _____________; // 
    }
    
    return head;
}

void printList(Node* head) {
    Node* current = head;
    while (current != nullptr) {
        cout << current->data;
        if (current->next != nullptr) {
            cout << " <-> ";
        }
        current = current->next;
    }
    cout << endl;
}

int main() {
    Node* head = new Node(1);
    Node* node2 = new Node(2);
    Node* node3 = new Node(3);
    Node* node4 = new Node(4);
    
    head->next = node2;
    node2->prev = head;
    node2->next = node3;
    node3->prev = node2;
    node3->next = node4;
    node4->prev = node3;
    
    cout << "Lista original:" << endl;
    printList(head);
    
    head = reverseDoublyLinkedList(head);
    
    cout << "Lista invertida:" << endl;
    printList(head);
    
    return 0;
}


// crear un programa que vaya eliminando los nodos de una lista enlazada circular en base a sus posiciones , ejemplo:

// Borrar en dos en dos. se borra el primer nodo, se saltan dos posiciones se borra el nodo correspondiente.

// se debe de ingresar la cantidad de nodos que se requieren saltar antes de empezar la secuencia de eliminacion.
// mostrar el dato del nodo que se vaya a borrar 

#include <iostream>

using namespace std;

struct Node {
    int data;
    Node* next;
    
    Node(int val) {
        data = val;
        next = nullptr;
    }
};

void solveJosephus(Node* head, int k) {
    if (!head) return;
    
    Node* current = head;
    Node* prev = nullptr;
    
    while (current->next != current) {
        for (int i = 0; i < k; i++) {
            prev = current;
            current = _____________; 
        }
        
        cout << "Borrado: " << current->data << endl;
        
        prev->next = current->next;
        Node* temp = current;
        current = _____________; 
        delete temp;
    }
    
    cout << "Borrado: " << current->data << endl;
    delete current;
}

int main() {
    int n = 5;
    int k;
    
    cout << "Ingrese la cantidad de nodos a saltar: ";
    cin >> k;
    
    Node* head = new Node(1);
    Node* tail = head;
    for (int i = 2; i <= n; i++) {
        tail->next = new Node(i);
        tail = tail->next;
    }
    tail->next = head;
    
    solveJosephus(head, k);
    
    return 0;
}