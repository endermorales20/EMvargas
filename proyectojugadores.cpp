#include <iostream>
#include <cstring>
#include <limits>
#include <cstdlib> 
#include <clocale>

using namespace std;


// 1. estructura y datos de la parte de jugadores


struct Torneo {
    char nombre[100];       
    char deporte[50];       
    char formato[20];       
    char fechaInicio[11];   
    char fechaFin[11];      
};

struct Jugador {
    int  id;                
    int  idEquipo;          
    char nombre[100];       
    char cedula[20];        
    char posicion[20];      
    int  edad;              
    int  numeroDorsal;      
    char fechaRegistro[11]; 
};

struct SistemaDeportivo {
    Torneo torneo;              
    Jugador* jugadores;
    int      numJugadores;
    int      capacidadJugadores;
    int siguienteIdJugador;
};


// auxiliares minimos


bool contieneSubcadena(const char* cadena, const char* subcadena) {
    if (!cadena || !subcadena) return false;
    if (subcadena[0] == '\0') return true;
    for (int i = 0; cadena[i] != '\0'; i++) {
        int j = 0;
        while (cadena[i + j] != '\0' && subcadena[j] != '\0' && 
               tolower(cadena[i + j]) == tolower(subcadena[j])) {
            j++;
        }
        if (subcadena[j] == '\0') return true;
    }
    return false;
}

void leerEntero(int &variable) {
    while (!(cin >> variable)) {
        cout << "Entrada invalida. Ingrese un numero entero: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

bool confirmarAccion(const char* mensaje) {
    char op;
    cout << mensaje << " (S/N): ";
    cin >> op;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return (op == 'S' || op == 's');
}

// 2. INICIALIZACIÓN Y MEMORIA - JUGADORES


void inicializarSistema(SistemaDeportivo* s, Torneo torneo) {
    s->torneo = torneo;
    s->capacidadJugadores = 4;
    s->numJugadores = 0;
    s->jugadores = new Jugador[s->capacidadJugadores];
    s->siguienteIdJugador = 1;
}

void redimensionarJugadores(SistemaDeportivo* s) {
    int nuevaCapacidad = s->capacidadJugadores * 2;
    Jugador* nuevoArray = new Jugador[nuevaCapacidad];
    for (int i = 0; i < s->numJugadores; i++) {
        nuevoArray[i] = s->jugadores[i];
    }
    delete[] s->jugadores;
    s->jugadores = nuevoArray;
    s->capacidadJugadores = nuevaCapacidad;
}

void liberarSistema(SistemaDeportivo* s) {
    if (s->jugadores) {
        delete[] s->jugadores;
        s->jugadores = nullptr;
    }
    s->numJugadores = 0;
}


// 3. CAPA DE LÓGICA - JUGADORES


Jugador* agregarJugador(SistemaDeportivo* s, int idEquipo, const char* nombre, const char* cedula, const char* posicion, int edad, int dorsal) {
    // Validar que la cédula no esté repetida en el torneo
    for (int i = 0; i < s->numJugadores; i++) {
        if (strcmp(s->jugadores[i].cedula, cedula) == 0) {
            return nullptr; 
        }
    }
    
    if (s->numJugadores == s->capacidadJugadores) {
        redimensionarJugadores(s);
    }
    
    Jugador& nuevo = s->jugadores[s->numJugadores];
    nuevo.id = s->siguienteIdJugador++;
    nuevo.idEquipo = idEquipo;
    strncpy(nuevo.nombre, nombre, 99);
    strncpy(nuevo.cedula, cedula, 19);
    strncpy(nuevo.posicion, posicion, 19);
    nuevo.edad = edad;
    nuevo.numeroDorsal = dorsal;
    strncpy(nuevo.fechaRegistro, "2026-06-11", 10);
    
    s->numJugadores++;
    return &nuevo;
}

Jugador* buscarJugadorPorID(SistemaDeportivo* s, int id) {
    for (int i = 0; i < s->numJugadores; i++) {
        if (s->jugadores[i].id == id) {
            return &s->jugadores[i];
        }
    }
    return nullptr;
}

Jugador** buscarJugadoresPorNombre(SistemaDeportivo* s, const char* subcadena, int* cantidad) {
    *cantidad = 0;
    for (int i = 0; i < s->numJugadores; i++) {
        if (contieneSubcadena(s->jugadores[i].nombre, subcadena)) {
            (*cantidad)++;
        }
    }
    if (*cantidad == 0) return nullptr;
    
    Jugador** resultado = new Jugador*[*cantidad];
    int index = 0;
    for (int i = 0; i < s->numJugadores; i++) {
        if (contieneSubcadena(s->jugadores[i].nombre, subcadena)) {
            resultado[index++] = &s->jugadores[i];
        }
    }
    return resultado;
}

bool actualizarJugador(SistemaDeportivo* s, int id, Jugador actualizado) {
    Jugador* j = buscarJugadorPorID(s, id);
    if (!j) return false;
    strncpy(j->nombre, actualizado.nombre, 99);
    strncpy(j->posicion, actualizado.posicion, 19);
    j->edad = actualizado.edad;
    j->numeroDorsal = actualizado.numeroDorsal;
    return true;
}

bool eliminarJugador(SistemaDeportivo* s, int id) {
    int index = -1;
    for (int i = 0; i < s->numJugadores; i++) {
        if (s->jugadores[i].id == id) {
            index = i;
            break;
        }
    }
    if (index == -1) return false;
    
    for (int i = index; i < s->numJugadores - 1; i++) {
        s->jugadores[i] = s->jugadores[i + 1];
    }
    s->numJugadores--;
    return true;
}


// 4. PRESENTACIÓN - JUGADORES


void mostrarJugador(Jugador* j) {
    if (!j) return;
    cout << "ID: " << j->id << " | Equipo ID: " << j->idEquipo 
         << " | Nombre: " << j->nombre << " | C.I: " << j->cedula 
         << " | Pos: " << j->posicion << " | Dorsal: " << j->numeroDorsal << endl;
}

void menuRegistrarJugador(SistemaDeportivo* s) {
    char nombre[100], cedula[20], posicion[20];
    int idEquipo, edad, dorsal;
    
    cout << "=== REGISTRAR JUGADOR ===\n";
    cout << "Ingrese ID del equipo al que pertenece: ";
    leerEntero(idEquipo);
    cout << "Ingrese nombre del jugador: ";
    cin.getline(nombre, 99);
    cout << "Ingrese cedula de identidad: ";
    cin.getline(cedula, 19);
    cout << "Ingrese posicion de juego: ";
    cin.getline(posicion, 19);
    cout << "Ingrese edad: ";
    leerEntero(edad);
    cout << "Ingrese numero de dorsal (#): ";
    leerEntero(dorsal);
    
    if (!confirmarAccion("Desea guardar este jugador?")) return;
    
    Jugador* res = agregarJugador(s, idEquipo, nombre, cedula, posicion, edad, dorsal);
    if (res) {
        cout << "Jugador registrado exitosamente!\n";
        mostrarJugador(res);
    } else {
        cout << "ERROR: La cedula ya se encuentra registrada en el sistema.\n";
    }
}

void menuBuscarJugador(SistemaDeportivo* s) {
    cout << "=== BUSCAR JUGADOR ===\n";
    cout << "1. Buscar por ID\n";
    cout << "2. Buscar por nombre\n";
    cout << "Seleccione una opcion: ";
    int op;
    leerEntero(op);
    
    if (op == 1) {
        cout << "Ingrese ID del jugador: ";
        int id;
        leerEntero(id);
        Jugador* j = buscarJugadorPorID(s, id);
        if (j) mostrarJugador(j);
        else cout << "ERROR: Jugador no encontrado.\n";
    } else if (op == 2) {
        char sub[100];
        cout << "Ingrese nombre a buscar: ";
        cin.getline(sub, 99);
        int cant = 0;
        Jugador** res = buscarJugadoresPorNombre(s, sub, &cant);
        if (res) {
            for (int i = 0; i < cant; i++) mostrarJugador(res[i]);
            delete[] res;
        } else {
            cout << "No se encontraron coincidencias.\n";
        }
    }
}

void menuListarJugadores(SistemaDeportivo* s) {
    cout << "=== LISTADO DE JUGADORES ===\n";
    if (s->numJugadores == 0) {
        cout << "No hay jugadores registrados.\n";
        return;
    }
    for (int i = 0; i < s->numJugadores; i++) {
        mostrarJugador(&s->jugadores[i]);
    }
}


// 5. FUNCIÓN PRINCIPAL


int main() {
    setlocale(LC_ALL, "spanish");
    
    Torneo t;
    strcpy(t.nombre, "Torneo Universitario URU 2026");

    SistemaDeportivo sistema;
    inicializarSistema(&sistema, t);

    int opcion;
    do {
        system("cls");
        cout << "==================================================\n";
        cout << "       GESTION DE JUGADORES - URU 2026\n";
        cout << "==================================================\n";
        cout << "1. Registrar Jugador\n";
        cout << "2. Buscar Jugador\n";
        cout << "3. Listar todos los Jugadores\n";
        cout << "4. Salir del Modulo\n";
        cout << "==================================================\n";
        cout << "Seleccione una opcion: ";
        leerEntero(opcion);

        system("cls");
        switch(opcion) {
            case 1: menuRegistrarJugador(&sistema); break;
            case 2: menuBuscarJugador(&sistema); break;
            case 3: menuListarJugadores(&sistema); break;
            case 4: cout << "Saliendo del modulo...\n"; break;
            default: cout << "Opcion invalida.\n"; break;
        }
        if (opcion != 4) {
            cout << "\nPresione Enter para continuar...";
            cin.get();
        }
    } while (opcion != 4);

    liberarSistema(&sistema);
    return 0;
}