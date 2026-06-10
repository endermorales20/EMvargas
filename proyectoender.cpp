#include <iostream>
#include <cstring>
#include <limits>
#include <clocale>
#include <windows.h>

using namespace std;


// 1. Estructura de datos paso uno

struct Torneo {
    char nombre[100];       
    char deporte[50];       
    char formato[20];       
    char fechaInicio[11];   
    char fechaFin[11];      
};

struct Equipo {
    int  id;                
    char nombre[100];       
    char ciudad[100];       
    char entrenador[100];   
    int  puntos;            
    int  victorias;         
    int  empates;           
    int  derrotas;          
    int  puntosAFavor;       
    int  puntosEnContra;     
    char fechaRegistro[11]; 
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

struct Partido {
    int  id;                  
    int  idEquipoLocal;       
    int  idEquipoVisitante;   
    int  puntosLocal;          
    int  puntosVisitante;      
    char fecha[11];           
    char estado[12];          
    char descripcion[200];    
};

struct SistemaDeportivo {
    Torneo torneo;              
    Equipo* equipos;
    int      numEquipos;
    int      capacidadEquipos;
    Jugador* jugadores;
    int      numJugadores;
    int      capacidadJugadores;
    Partido* partidos;
    int      numPartidos;
    int      capacidadPartidos;
    int siguienteIdEquipo;
    int siguienteIdJugador;
    int siguienteIdPartido;
};


// minimo de cadenas y la lectura

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


// 2. inicio y liberacion

void inicializarSistema(SistemaDeportivo* s, Torneo torneo) {
    s->torneo = torneo;
    
    s->capacidadEquipos = 4;
    s->numEquipos = 0;
    s->equipos = new Equipo[s->capacidadEquipos];
    
    s->capacidadJugadores = 4;
    s->numJugadores = 0;
    s->jugadores = new Jugador[s->capacidadJugadores];
    
    s->capacidadPartidos = 4;
    s->numPartidos = 0;
    s->partidos = new Partido[s->capacidadPartidos];
    
    s->siguienteIdEquipo = 1;
    s->siguienteIdJugador = 1;
    s->siguienteIdPartido = 1;
}

void redimensionarEquipos(SistemaDeportivo* s) {
    int nuevaCapacidad = s->capacidadEquipos * 2;
    Equipo* nuevoArray = new Equipo[nuevaCapacidad];
    for (int i = 0; i < s->numEquipos; i++) {
        nuevoArray[i] = s->equipos[i];
    }
    delete[] s->equipos;
    s->equipos = nuevoArray;
    s->capacidadEquipos = nuevaCapacidad;
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

void redimensionarPartidos(SistemaDeportivo* s) {
    int nuevaCapacidad = s->capacidadPartidos * 2;
    Partido* nuevoArray = new Partido[nuevaCapacidad];
    for (int i = 0; i < s->numPartidos; i++) {
        nuevoArray[i] = s->partidos[i];
    }
    delete[] s->partidos;
    s->partidos = nuevoArray;
    s->capacidadPartidos = nuevaCapacidad;
}

void liberarSistema(SistemaDeportivo* s) {
    if (s->equipos) delete[] s->equipos;
    if (s->jugadores) delete[] s->jugadores;
    if (s->partidos) delete[] s->partidos;
    s->numEquipos = 0; s->numJugadores = 0; s->numPartidos = 0;
}


// programa inicial

int main() {
    setlocale(LC_ALL, "spanish");
    
    Torneo t;
    strcpy(t.nombre, "Torneo Universitario URU 2026");
    strcpy(t.deporte, "Fútbol Sala");
    strcpy(t.formato, "Liga");
    strcpy(t.fechaInicio, "2026-06-01");
    strcpy(t.fechaFin, "2026-07-15");

    SistemaDeportivo sistema;
    inicializarSistema(&sistema, t);

    int opcion;
    do {
        system("cls");
        cout << "==================================================\n";
        cout << "   SISTEMA DEPORTIVO - " << sistema.torneo.nombre << "\n";
        cout << "==================================================\n";
        cout << "1. Modulo de Equipos (Bloqueado hasta manana)\n";
        cout << "2. Modulo de Jugadores (Bloqueado hasta pasado manana)\n";
        cout << "3. Modulo de Partidos (Bloqueado hasta pasado manana)\n";
        cout << "4. Salir del Sistema\n";
        cout << "==================================================\n";
        cout << "Seleccione una opcion: ";
        leerEntero(opcion);

        if(opcion >= 1 && opcion <= 3) {
            cout << "\n[INFO] Modulo en desarrollo incremental. Se desbloqueara en el commit programado.\n";
            cout << "Presione Enter para continuar...";
            cin.get();
        }
    } while (opcion != 4);

    liberarSistema(&sistema);
    cout << "\nMemoria liberada. Sistema cerrado con exito.\n";
    return 0;
}