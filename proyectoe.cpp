#include <iostream>
#include <cstring>
#include <limits>
#include <clocale>
#include <windows.h>

using namespace std;

// ============================================================================
// 1. ESTRUCTURAS DE DATOS OBLIGATORIAS
// ============================================================================

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

// ============================================================================
// AUXILIARES MINIMOS DE CADENAS (Reemplazo manual para evitar dependencias)
// ============================================================================
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

// ============================================================================
// 2. INICIALIZACION, REDIMENSIONAMIENTO Y LIBERACION (SISTEMA GLOBAL)
// ============================================================================

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
    if (s->equipos) {
        delete[] s->equipos;
        s->equipos = nullptr;
    }
    if (s->jugadores) {
        delete[] s->jugadores;
        s->jugadores = nullptr;
    }
    if (s->partidos) {
        delete[] s->partidos;
        s->partidos = nullptr;
    }
    s->numEquipos = 0;
    s->numJugadores = 0;
    s->numPartidos = 0;
}

// Auxiliar de lectura numerica segura en presentacion
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

// ============================================================================
// 3. CAPA DE LOGICA - EQUIPOS
// ============================================================================

Equipo* agregarEquipo(SistemaDeportivo* s, const char* nombre, const char* ciudad, const char* entrenador) {
    for (int i = 0; i < s->numEquipos; i++) {
        if (strcmp(s->equipos[i].nombre, nombre) == 0) {
            return nullptr; 
        }
    }
    if (s->numEquipos == s->capacidadEquipos) {
        redimensionarEquipos(s);
    }
    
    Equipo& nuevo = s->equipos[s->numEquipos];
    nuevo.id = s->siguienteIdEquipo++;
    strncpy(nuevo.nombre, nombre, 99);
    strncpy(nuevo.ciudad, ciudad, 99);
    strncpy(nuevo.entrenador, entrenador, 99);
    nuevo.puntos = 0;
    nuevo.victorias = 0;
    nuevo.empates = 0;
    nuevo.derrotas = 0;
    nuevo.puntosAFavor = 0;
    nuevo.puntosEnContra = 0;
    strncpy(nuevo.fechaRegistro, "2026-06-09", 10); 
    
    s->numEquipos++;
    return &nuevo;
}

Equipo* buscarEquipoPorID(SistemaDeportivo* s, int id) {
    for (int i = 0; i < s->numEquipos; i++) {
        if (s->equipos[i].id == id) {
            return &s->equipos[i];
        }
    }
    return nullptr;
}

Equipo** buscarEquiposPorNombre(SistemaDeportivo* s, const char* subcadena, int* cantidad) {
    *cantidad = 0;
    for (int i = 0; i < s->numEquipos; i++) {
        if (contieneSubcadena(s->equipos[i].nombre, subcadena)) {
            (*cantidad)++;
        }
    }
    if (*cantidad == 0) return nullptr;
    
    Equipo** resultado = new Equipo*[*cantidad];
    int index = 0;
    for (int i = 0; i < s->numEquipos; i++) {
        if (contieneSubcadena(s->equipos[i].nombre, subcadena)) {
            resultado[index++] = &s->equipos[i];
        }
    }
    return resultado;
}

Equipo** listarEquipos(SistemaDeportivo* s, int* cantidad) {
    *cantidad = s->numEquipos;
    if (s->numEquipos == 0) return nullptr;
    
    Equipo** resultado = new Equipo*[s->numEquipos];
    for (int i = 0; i < s->numEquipos; i++) {
        resultado[i] = &s->equipos[i];
    }
    return resultado;
}

Equipo** generarTablaPosiciones(SistemaDeportivo* s, int* cantidad) {
    *cantidad = s->numEquipos;
    if (s->numEquipos == 0) return nullptr;
    
    Equipo** tabla = new Equipo*[s->numEquipos];
    for (int i = 0; i < s->numEquipos; i++) {
        tabla[i] = &s->equipos[i];
    }
    
    for (int i = 0; i < s->numEquipos - 1; i++) {
        for (int j = 0; j < s->numEquipos - i - 1; j++) {
            bool intercambiar = false;
            int ptsJ = tabla[j]->puntos;
            int ptsJ1 = tabla[j+1]->puntos;
            
            if (ptsJ < ptsJ1) {
                intercambiar = true;
            } else if (ptsJ == ptsJ1) {
                int difJ = tabla[j]->puntosAFavor - tabla[j]->puntosEnContra;
                int difJ1 = tabla[j+1]->puntosAFavor - tabla[j+1]->puntosEnContra;
                if (difJ < difJ1) {
                    intercambiar = true;
                } else if (difJ == difJ1) {
                    if (tabla[j]->puntosAFavor < tabla[j+1]->puntosAFavor) {
                        intercambiar = true;
                    }
                }
            }
            if (intercambiar) {
                Equipo* temp = tabla[j];
                tabla[j] = tabla[j+1];
                tabla[j+1] = temp;
            }
        }
    }
    return tabla;
}

bool actualizarEquipo(SistemaDeportivo* s, int id, Equipo equipoActualizado) {
    Equipo* eq = buscarEquipoPorID(s, id);
    if (!eq) return false;
    strncpy(eq->nombre, equipoActualizado.nombre, 99);
    strncpy(eq->ciudad, equipoActualizado.ciudad, 99);
    strncpy(eq->entrenador, equipoActualizado.entrenador, 99);
    return true;
}

bool eliminarEquipo(SistemaDeportivo* s, int id) {
    int index = -1;
    for (int i = 0; i < s->numEquipos; i++) {
        if (s->equipos[i].id == id) {
            index = i;
            break;
        }
    }
    if (index == -1) return false;
    
    for (int i = 0; i < s->numPartidos; i++) {
        if (s->partidos[i].idEquipoLocal == id || s->partidos[i].idEquipoVisitante == id) {
            return false; 
        }
    }
    
    // Remover del array compactando datos
    for (int i = index; i < s->numEquipos - 1; i++) {
        s->equipos[i] = s->equipos[i + 1];
    }
    s->numEquipos--;
    return true;
}

// ============================================================================
// 4. CAPA DE PRESENTACION - EQUIPOS
// ============================================================================

void mostrarEquipo(Equipo* equipo) {
    if (!equipo) return;
    cout << "ID: " << equipo->id << " | Nombre: " << equipo->nombre 
         << " | Ciudad: " << equipo->ciudad << " | Entrenador: " << equipo->entrenador 
         << " | PTS: " << equipo->puntos << endl;
}

void mostrarListaEquipos(Equipo** equipos, int cantidad) {
    if (!equipos || cantidad == 0) {
        cout << "No hay equipos para mostrar." << endl;
        return;
    }
    for (int i = 0; i < cantidad; i++) {
        mostrarEquipo(equipos[i]);
    }
}

void menuRegistrarEquipo(SistemaDeportivo* s) {
    char nombre[100], ciudad[100], entrenador[100];
    cout << "=== REGISTRAR EQUIPO ===" << endl;
    cout << "Ingrese nombre del equipo (o 'CANCELAR'): ";
    cin.getline(nombre, 99);
    if (strcmp(nombre, "CANCELAR") == 0) return;
    
    if (strlen(nombre) == 0) {
        cout << "ERROR: El nombre no puede estar vacio." << endl;
        return;
    }
    cout << "Ingrese ciudad: ";
    cin.getline(ciudad, 99);
    cout << "Ingrese entrenador: ";
    cin.getline(entrenador, 99);
    
    if (!confirmarAccion("Desea guardar el nuevo registro?")) return;
    
    Equipo* res = agregarEquipo(s, nombre, ciudad, entrenador);
    if (res) {
        cout << "Equipo registrado exitosamente!" << endl;
        mostrarEquipo(res);
    } else {
        cout << "ERROR: Ya existe un equipo con el nombre '" << nombre << "'." << endl;
    }
}

void menuBuscarEquipo(SistemaDeportivo* s) {
    cout << "=== BUSCAR EQUIPO ===" << endl;
    cout << "1. Buscar por ID" << endl;
    cout << "2. Buscar por nombre parcial" << endl;
    cout << "Seleccione una opcion: ";
    int op;
    leerEntero(op);
    
    if (op == 1) {
        cout << "Ingrese ID a buscar: ";
        int id;
        leerEntero(id);
        Equipo* eq = buscarEquipoPorID(s, id);
        if (eq) mostrarEquipo(eq);
        else cout << "ERROR: No existe ningun equipo con ID " << id << "." << endl;
    } else if (op == 2) {
        char sub[100];
        cout << "Ingrese subcadena de nombre: ";
        cin.getline(sub, 99);
        int cant = 0;
        Equipo** res = buscarEquiposPorNombre(s, sub, &cant);
        if (res) {
            mostrarListaEquipos(res, cant);
            delete[] res;
        } else {
            cout << "No se encontraron coincidencias." << endl;
        }
    }
}

void menuActualizarEquipo(SistemaDeportivo* s) {
    cout << "=== ACTUALIZAR EQUIPO ===" << endl;
    cout << "Ingrese ID del equipo a editar: ";
    int id;
    leerEntero(id);
    Equipo* eq = buscarEquipoPorID(s, id);
    if (!eq) {
        cout << "ERROR: No existe ningun equipo con ID " << id << "." << endl;
        return;
    }
    
    Equipo temp = *eq;
    cout << "Nombre actual [" << eq->nombre << "]. Ingrese nuevo: ";
    cin.getline(temp.nombre, 99);
    cout << "Ciudad actual [" << eq->ciudad << "]. Ingrese nueva: ";
    cin.getline(temp.ciudad, 99);
    cout << "Entrenador actual [" << eq->entrenador << "]. Ingrese nuevo: ";
    cin.getline(temp.entrenador, 99);
    
    if (!confirmarAccion("Confirmar modificaciones?")) return;
    
    if (actualizarEquipo(s, id, temp)) {
        cout << "Equipo actualizado correctamente." << endl;
    }
}

void menuEliminarEquipo(SistemaDeportivo* s) {
    cout << "=== ELIMINAR EQUIPO ===" << endl;
    cout << "Ingrese ID del equipo a eliminar: ";
    int id;
    leerEntero(id);
    
    Equipo* eq = buscarEquipoPorID(s, id);
    if (!eq) {
        cout << "ERROR: No existe ningun equipo con ID " << id << "." << endl;
        return;
    }
    
    // Contar si posee dependencias para mandar advertencia de negocio
    int asociados = 0;
    for (int i = 0; i < s->numPartidos; i++) {
        if (s->partidos[i].idEquipoLocal == id || s->partidos[i].idEquipoVisitante == id) {
            asociados++;
        }
    }
    if (asociados > 0) {
        cout << "ADVERTENCIA: El equipo tiene " << asociados << " partidos asociados. Desea eliminarlo igualmente? (S/N)" << endl;
        if (!confirmarAccion("Confirmar forzado?")) return;
    } else {
        if (!confirmarAccion("Esta seguro de eliminar este equipo?")) return;
    }
    
    if (eliminarEquipo(s, id)) {
        cout << "Equipo removido exitosamente." << endl;
    } else {
        cout << "ERROR: No se pudo eliminar el equipo debido a la integridad de datos referenciales." << endl;
    }
}

void menuListarEquipos(SistemaDeportivo* s) {
    cout << "=== LISTADO DE EQUIPOS ===" << endl;
    int cant = 0;
    Equipo** lista = listarEquipos(s, &cant);
    if (lista) {
        mostrarListaEquipos(lista, cant);
        delete[] lista;
    } else {
        cout << "No hay equipos registrados en el sistema." << endl;
    }
}

void mostrarTablaPosiciones(SistemaDeportivo* s) {
    int cant = 0;
    Equipo** tabla = generarTablaPosiciones(s, &cant);
    cout << "╔══════════════════════════════════════════════════════════════════════╗" << endl;
    cout << "║                      TABLA DE POSICIONES                             ║" << endl;
    cout << "║                      " << s->torneo.nombre << "                              ║" << endl;
    cout << "╠════╦══════════════════╦═════╦═══╦═══╦═══╦════╦════╦════╣" << endl;
    cout << "║ #  ║ Equipo           ║ PTS ║ J ║ G ║ E ║ D  ║ GF ║ GC ║" << endl;
    cout << "╠════╬══════════════════╬═════╬═══╬═══╬═══╬════╬════╬════╣" << endl;
    if (!tabla || cant == 0) {
        cout << "║ -- ║ Sin registros    ║  0  ║ 0 ║ 0 ║ 0 ║ 0  ║ 0  ║ 0  ║" << endl;
    } else {
        for (int i = 0; i < cant; i++) {
            int jugados = tabla[i]->victorias + tabla[i]->empates + tabla[i]->derrotas;
            printf("║ %2d ║ %-16.16s ║ %3d ║ %1d ║ %1d ║ %1d ║ %1d  ║ %2d ║ %2d ║\n",
                   i+1, tabla[i]->nombre, tabla[i]->puntos, jugados,
                   tabla[i]->victorias, tabla[i]->empates, tabla[i]->derrotas,
                   tabla[i]->puntosAFavor, tabla[i]->puntosEnContra);
        }
    }
    cout << "╚════╩══════════════════╩═════╩═══╩═══╩═══╩════╩════╩════╝" << endl;
    if (tabla) delete[] tabla;
}

// ============================================================================
// 5. CAPA DE LOGICA - JUGADORES
// ============================================================================

Jugador* agregarJugador(SistemaDeportivo* s, int idEquipo, const char* nombre,
                        const char* cedula, const char* posicion, int edad, int numeroDorsal) {
    // Validaciones obligatorias de logica empresarial
    bool equipoExiste = false;
    for (int i = 0; i < s->numEquipos; i++) {
        if (s->equipos[i].id == idEquipo) equipoExiste = true;
    }
    if (!equipoExiste) return nullptr;
    
    for (int i = 0; i < s->numJugadores; i++) {
        if (strcmp(s->jugadores[i].cedula, cedula) == 0) return nullptr;
    }
    
    for (int i = 0; i < s->numJugadores; i++) {
        if (s->jugadores[i].idEquipo == idEquipo && s->jugadores[i].numeroDorsal == numeroDorsal) {
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
    nuevo.numeroDorsal = numeroDorsal;
    strncpy(nuevo.fechaRegistro, "2026-06-09", 10);
    
    s->numJugadores++;
    return &nuevo;
}

Jugador* buscarJugadorPorID(SistemaDeportivo* s, int id) {
    for (int i = 0; i < s->numJugadores; i++) {
        if (s->jugadores[i].id == id) return &s->jugadores[i];
    }
    return nullptr;
}

Jugador** buscarJugadoresPorNombre(SistemaDeportivo* s, const char* subcadena, int* cantidad) {
    *cantidad = 0;
    for (int i = 0; i < s->numJugadores; i++) {
        if (contieneSubcadena(s->jugadores[i].nombre, subcadena)) (*cantidad)++;
    }
    if (*cantidad == 0) return nullptr;
    
    Jugador** res = new Jugador*[*cantidad];
    int index = 0;
    for (int i = 0; i < s->numJugadores; i++) {
        if (contieneSubcadena(s->jugadores[i].nombre, subcadena)) {
            res[index++] = &s->jugadores[i];
        }
    }
    return res;
}

Jugador** listarJugadoresPorEquipo(SistemaDeportivo* s, int idEquipo, int* cantidad) {
    *cantidad = 0;
    for (int i = 0; i < s->numJugadores; i++) {
        if (s->jugadores[i].idEquipo == idEquipo) (*cantidad)++;
    }
    if (*cantidad == 0) return nullptr;
    
    Jugador** res = new Jugador*[*cantidad];
    int index = 0;
    for (int i = 0; i < s->numJugadores; i++) {
        if (s->jugadores[i].idEquipo == idEquipo) {
            res[index++] = &s->jugadores[i];
        }
    }
    return res;
}

Jugador** listarJugadores(SistemaDeportivo* s, int* cantidad) {
    *cantidad = s->numJugadores;
    if (s->numJugadores == 0) return nullptr;
    
    Jugador** res = new Jugador*[s->numJugadores];
    for (int i = 0; i < s->numJugadores; i++) {
        res[i] = &s->jugadores[i];
    }
    return res;
}

bool actualizarJugador(SistemaDeportivo* s, int id, Jugador jugadorActualizado) {
    Jugador* j = buscarJugadorPorID(s, id);
    if (!j) return false;
    
    strncpy(j->nombre, jugadorActualizado.nombre, 99);
    strncpy(j->posicion, jugadorActualizado.posicion, 19);
    j->edad = jugadorActualizado.edad;
    j->numeroDorsal = jugadorActualizado.numeroDorsal;
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

// ============================================================================
// 6. CAPA DE PRESENTACION - JUGADORES
// ============================================================================

void mostrarJugador(Jugador* jugador, SistemaDeportivo* s) {
    if (!jugador) return;
    Equipo* eq = buscarEquipoPorID(s, jugador->idEquipo);
    const char* nombreEq = eq ? eq->nombre : "Desconocido";
    cout << "ID: " << jugador->id << " | Nombre: " << jugador->nombre 
         << " | Equipo: " << nombreEq << " | CI: " << jugador->cedula 
         << " | Posicion: " << jugador->posicion << " | Dorsal: " << jugador->numeroDorsal << endl;
}

void mostrarListaJugadores(Jugador** jugadores, int cantidad, SistemaDeportivo* s) {
    if (!jugadores || cantidad == 0) {
        cout << "No hay jugadores registrados en la vista." << endl;
        return;
    }
    cout << "╔════╦══════════════════╦══════════════╦═══════════════╦═════╦═════════╗" << endl;
    cout << "║ ID ║ Nombre           ║ Equipo       ║ Posicion      ║ Edad║ Dorsal  ║" << endl;
    cout << "╠════╬══════════════════╬══════════════╬═══════════════╬═════╬═════════╣" << endl;
    for (int i = 0; i < cantidad; i++) {
        Equipo* eq = buscarEquipoPorID(s, jugadores[i]->idEquipo);
        const char* nEq = eq ? eq->nombre : "N/A";
        printf("║ %2d ║ %-16.16s ║ %-12.12s ║ %-13.13s ║ %3d ║   %2d    ║\n",
               jugadores[i]->id, jugadores[i]->nombre, nEq, 
               jugadores[i]->posicion, jugadores[i]->edad, jugadores[i]->numeroDorsal);
    }
    cout << "╚════╩══════════════════╩══════════════╩═══════════════╩═════╩═════════╝" << endl;
}

void menuRegistrarJugador(SistemaDeportivo* s) {
    cout << "=== REGISTRAR JUGADOR ===" << endl;
    char nombre[100], cedula[20], posicion[20];
    int idEquipo, edad, dorsal;
    
    cout << "Ingrese nombre del jugador (o 'CANCELAR'): ";
    cin.getline(nombre, 99);
    if (strcmp(nombre, "CANCELAR") == 0) return;
    
    cout << "Ingrese ID del Equipo asignado: ";
    leerEntero(idEquipo);
    Equipo* eq = buscarEquipoPorID(s, idEquipo);
    if (!eq) {
        cout << "ERROR: No existe ningun equipo con ID " << idEquipo << "." << endl;
        return;
    }
    
    cout << "Ingrese cedula: ";
    cin.getline(cedula, 19);
    // Verificacion temprana en capa de presentacion
    for (int i = 0; i < s->numJugadores; i++) {
        if (strcmp(s->jugadores[i].cedula, cedula) == 0) {
            cout << "ERROR: La cedula '" << cedula << "' ya esta registrada." << endl;
            return;
        }
    }
    
    cout << "Ingrese posicion (PORTERO, DEFENSA, MEDIOCAMPISTA, DELANTERO): ";
    cin.getline(posicion, 19);
    if (strcmp(posicion, "PORTERO") != 0 && strcmp(posicion, "DEFENSA") != 0 &&
        strcmp(posicion, "MEDIOCAMPISTA") != 0 && strcmp(posicion, "DELANTERO") != 0) {
        cout << "ERROR: Posicion no valida." << endl;
        return;
    }
    
    cout << "Ingrese edad (14-50): ";
    leerEntero(edad);
    if (edad < 14 || edad > 50) {
        cout << "ERROR: Rango de edad invalido." << endl;
        return;
    }
    
    cout << "Ingrese numero dorsal (1-99): ";
    leerEntero(dorsal);
    if (dorsal < 1 || dorsal > 99) {
        cout << "ERROR: Dorsal fuera de rango." << endl;
        return;
    }
    
    for (int i = 0; i < s->numJugadores; i++) {
        if (s->jugadores[i].idEquipo == idEquipo && s->jugadores[i].numeroDorsal == dorsal) {
            cout << "ERROR: El dorsal " << dorsal << " ya esta en uso en el equipo '" << eq->nombre << "'." << endl;
            return;
        }
    }
    
    if (!confirmarAccion("Confirmar registro del jugador?")) return;
    
    Jugador* j = agregarJugador(s, idEquipo, nombre, cedula, posicion, edad, dorsal);
    if (j) cout << "Jugador registrado de manera correcta." << endl;
}

void menuBuscarJugador(SistemaDeportivo* s) {
    cout << "=== BUSCAR JUGADOR ===" << endl;
    cout << "Ingrese coincidencia parcial de nombre a rastrear: ";
    char sub[100];
    cin.getline(sub, 99);
    int cant = 0;
    Jugador** res = buscarJugadoresPorNombre(s, sub, &cant);
    if (res) {
        mostrarListaJugadores(res, cant, s);
        delete[] res;
    } else {
        cout << "Sin coincidencias encontradas." << endl;
    }
}

void menuActualizarJugador(SistemaDeportivo* s) {
    cout << "=== ACTUALIZAR JUGADOR ===" << endl;
    cout << "Ingrese ID del jugador: ";
    int id;
    leerEntero(id);
    Jugador* j = buscarJugadorPorID(s, id);
    if (!j) {
        cout << "ERROR: Jugador no existente." << endl;
        return;
    }
    Jugador temp = *j;
    cout << "Nombre [" << j->nombre << "]: ";
    cin.getline(temp.nombre, 99);
    cout << "Posicion [" << j->posicion << "]: ";
    cin.getline(temp.posicion, 19);
    cout << "Edad [" << j->edad << "]: ";
    leerEntero(temp.edad);
    cout << "Dorsal [" << j->numeroDorsal << "]: ";
    leerEntero(temp.numeroDorsal);
    
    if (!confirmarAccion("Aplicar cambios estructurales?")) return;
    if (actualizarJugador(s, id, temp)) cout << "Datos guardados." << endl;
}

void menuEliminarJugador(SistemaDeportivo* s) {
    cout << "=== ELIMINAR JUGADOR ===" << endl;
    cout << "Ingrese ID del jugador a dar de baja: ";
    int id;
    leerEntero(id);
    if (!buscarJugadorPorID(s, id)) {
        cout << "ERROR: Identificador invalido." << endl;
        return;
    }
    if (!confirmarAccion("Seguro de remover al atleta?")) return;
    if (eliminarJugador(s, id)) cout << "Jugador eliminado del sistema." << endl;
}

void menuListarJugadores(SistemaDeportivo* s) {
    cout << "=== LISTAR JUGADORES ===" << endl;
    cout << "1. Ver el universo completo" << endl;
    cout << "2. Filtrar especificamente por plantilla de equipo" << endl;
    int op;
    leerEntero(op);
    int cant = 0;
    if (op == 1) {
        Jugador** res = listarJugadores(s, &cant);
        if (res) {
            mostrarListaJugadores(res, cant, s);
            delete[] res;
        } else cout << "No hay jugadores registrados en el sistema." << endl;
    } else if (op == 2) {
        cout << "Ingrese ID del equipo: ";
        int idEq;
        leerEntero(idEq);
        Jugador** res = listarJugadoresPorEquipo(s, idEq, &cant);
        if (res) {
            mostrarListaJugadores(res, cant, s);
            delete[] res;
        } else cout << "Este equipo no posee jugadores o no existe." << endl;
    }
}

// ============================================================================
// 7. CAPA DE LOGICA - PARTIDOS
// ============================================================================

Partido* programarPartido(SistemaDeportivo* s, int idLocal, int idVisitante,
                          const char* fecha, const char* descripcion) {
    if (idLocal == idVisitante) return nullptr;
    
    bool localExiste = false, visitanteExiste = false;
    for (int i = 0; i < s->numEquipos; i++) {
        if (s->equipos[i].id == idLocal) localExiste = true;
        if (s->equipos[i].id == idVisitante) visitanteExiste = true;
    }
    if (!localExiste || !visitanteExiste) return nullptr;
    
    for (int i = 0; i < s->numPartidos; i++) {
        if (strcmp(s->partidos[i].estado, "PROGRAMADO") == 0 &&
            ((s->partidos[i].idEquipoLocal == idLocal && s->partidos[i].idEquipoVisitante == idVisitante) ||
             (s->partidos[i].idEquipoLocal == idVisitante && s->partidos[i].idEquipoVisitante == idLocal))) {
            return nullptr; 
        }
    }
    
    if (s->numPartidos == s->capacidadPartidos) {
        redimensionarPartidos(s);
    }
    
    Partido& nuevo = s->partidos[s->numPartidos];
    nuevo.id = s->siguienteIdPartido++;
    nuevo.idEquipoLocal = idLocal;
    nuevo.idEquipoVisitante = idVisitante;
    nuevo.puntosLocal = 0;
    nuevo.puntosVisitante = 0;
    strncpy(nuevo.fecha, fecha, 10);
    strncpy(nuevo.estado, "PROGRAMADO", 11);
    strncpy(nuevo.descripcion, descripcion, 199);
    
    s->numPartidos++;
    return &nuevo;
}

Partido* registrarResultado(SistemaDeportivo* s, int idPartido, int puntosLocal, int puntosVisitante) {
    Partido* p = nullptr;
    for (int i = 0; i < s->numPartidos; i++) {
        if (s->partidos[i].id == idPartido) {
            p = &s->partidos[i];
            break;
        }
    }
    if (!p || strcmp(p->estado, "JUGADO") == 0) return nullptr;
    
    p->puntosLocal = puntosLocal;
    p->puntosVisitante = puntosVisitante;
    strncpy(p->estado, "JUGADO", 11);
    
    Equipo* el = buscarEquipoPorID(s, p->idEquipoLocal);
    Equipo* ev = buscarEquipoPorID(s, p->idEquipoVisitante);
    
    if (el && ev) {
        el->puntosAFavor += puntosLocal;
        el->puntosEnContra += puntosVisitante;
        ev->puntosAFavor += puntosVisitante;
        ev->puntosEnContra += puntosLocal;
        
        if (puntosLocal > puntosVisitante) {
            el->puntos += 3;
            el->victorias += 1;
            ev->derrotas += 1;
        } else if (puntosLocal == puntosVisitante) {
            el->puntos += 1;
            ev->puntos += 1;
            el->empates += 1;
            ev->empates += 1;
        } else {
            ev->puntos += 3;
            ev->victorias += 1;
            el->derrotas += 1;
        }
    }
    return p;
}

Partido* buscarPartidoPorID(SistemaDeportivo* s, int id) {
    for (int i = 0; i < s->numPartidos; i++) {
        if (s->partidos[i].id == id) return &s->partidos[i];
    }
    return nullptr;
}

Partido** buscarPartidosPorEquipo(SistemaDeportivo* s, int idEquipo, int* cantidad) {
    *cantidad = 0;
    for (int i = 0; i < s->numPartidos; i++) {
        if (s->partidos[i].idEquipoLocal == idEquipo || s->partidos[i].idEquipoVisitante == idEquipo) {
            (*cantidad)++;
        }
    }
    if (*cantidad == 0) return nullptr;
    
    Partido** res = new Partido*[*cantidad];
    int index = 0;
    for (int i = 0; i < s->numPartidos; i++) {
        if (s->partidos[i].idEquipoLocal == idEquipo || s->partidos[i].idEquipoVisitante == idEquipo) {
            res[index++] = &s->partidos[i];
        }
    }
    return res;
}

Partido** listarPartidosPorEstado(SistemaDeportivo* s, const char* estado, int* cantidad) {
    *cantidad = 0;
    for (int i = 0; i < s->numPartidos; i++) {
        if (strcmp(s->partidos[i].estado, estado) == 0) (*cantidad)++;
    }
    if (*cantidad == 0) return nullptr;
    
    Partido** res = new Partido*[*cantidad];
    int index = 0;
    for (int i = 0; i < s->numPartidos; i++) {
        if (strcmp(s->partidos[i].estado, estado) == 0) {
            res[index++] = &s->partidos[i];
        }
    }
    return res;
}

Partido** listarPartidos(SistemaDeportivo* s, int* cantidad) {
    *cantidad = s->numPartidos;
    if (s->numPartidos == 0) return nullptr;
    
    Partido** res = new Partido*[s->numPartidos];
    for (int i = 0; i < s->numPartidos; i++) {
        res[i] = &s->partidos[i];
    }
    return res;
}

bool cancelarPartido(SistemaDeportivo* s, int idPartido) {
    Partido* p = buscarPartidoPorID(s, idPartido);
    if (!p || strcmp(p->estado, "CANCELADO") == 0) return false;
    
    if (strcmp(p->estado, "JUGADO") == 0) {
        Equipo* el = buscarEquipoPorID(s, p->idEquipoLocal);
        Equipo* ev = buscarEquipoPorID(s, p->idEquipoVisitante);
        if (el && ev) {
            el->puntosAFavor -= p->puntosLocal;
            el->puntosEnContra -= p->puntosVisitante;
            ev->puntosAFavor -= p->puntosVisitante;
            ev->puntosEnContra -= p->puntosLocal;
            
            if (p->puntosLocal > p->puntosVisitante) {
                el->puntos -= 3;
                el->victorias -= 1;
                ev->derrotas -= 1;
            } else if (p->puntosLocal == p->puntosVisitante) {
                el->puntos -= 1;
                ev->puntos -= 1;
                el->empates -= 1;
                ev->empates -= 1;
            } else {
                ev->puntos -= 3;
                ev->victorias -= 1;
                el->derrotas -= 1;
            }
        }
    }
    strncpy(p->estado, "CANCELADO", 11);
    return true;
}

// ============================================================================
// 8. CAPA DE PRESENTACION - PARTIDOS
// ============================================================================

void mostrarPartido(Partido* partido, SistemaDeportivo* s) {
    if (!partido) return;
    Equipo* el = buscarEquipoPorID(s, partido->idEquipoLocal);
    Equipo* ev = buscarEquipoPorID(s, partido->idEquipoVisitante);
    const char* nLocal = el ? el->nombre : "N/A";
    const char* nVisit = ev ? ev->nombre : "N/A";
    
    cout << "╔══════════════════════════════════════════════════╗" << endl;
    cout << "║              DETALLE DE PARTIDO                  ║" << endl;
    cout << "╠══════════════════════════════════════════════════╣" << endl;
    printf("║ ID Partido  : %-34d ║\n", partido->id);
    printf("║ Estado      : %-34s ║\n", partido->estado);
    printf("║ Fecha       : %-34s ║\n", partido->fecha);
    cout << "║                                                  ║" << endl;
    printf("║ %-20.20s  %2d - %-2d  %-20.20s ║\n", nLocal, partido->puntosLocal, partido->puntosVisitante, nVisit);
    cout << "║      (Local)                  (Visitante)        ║" << endl;
    cout << "║                                                  ║" << endl;
    printf("║ Notas: %-41.41s ║\n", partido->descripcion);
    cout << "╚══════════════════════════════════════════════════╝" << endl;
}

void mostrarListaPartidos(Partido** partidos, int cantidad, SistemaDeportivo* s) {
    if (!partidos || cantidad == 0) {
        cout << "No hay partidos que listar." << endl;
        return;
    }
    for (int i = 0; i < cantidad; i++) {
        mostrarPartido(partidos[i], s);
    }
}

void menuProgramarPartido(SistemaDeportivo* s) {
    cout << "=== PROGRAMAR PARTIDO ===" << endl;
    int idL, idV;
    char fecha[11], desc[200];
    
    cout << "Ingrese ID Equipo Local: ";
    leerEntero(idL);
    cout << "Ingrese ID Equipo Visitante: ";
    leerEntero(idV);
    
    if (idL == idV) {
        cout << "ERROR: No se puede programar un partido de un equipo contra si mismo." << endl;
        return;
    }
    if (!buscarEquipoPorID(s, idL) || !buscarEquipoPorID(s, idV)) {
        cout << "ERROR: Uno o ambos equipos no existen." << endl;
        return;
    }
    
    cout << "Fecha (YYYY-MM-DD): ";
    cin.getline(fecha, 10);
    cout << "Descripcion: ";
    cin.getline(desc, 199);
    
    if (!confirmarAccion("Confirmar planificacion de partido?")) return;
    
    Partido* p = programarPartido(s, idL, idV, fecha, desc);
    if (p) cout << "Partido agendado con exito." << endl;
    else cout << "ERROR: Validacion de negocio fallida (Posible partido ya programado entre si)." << endl;
}

void menuRegistrarResultado(SistemaDeportivo* s) {
    cout << "=== REGISTRAR RESULTADO ===" << endl;
    cout << "Ingrese ID del Partido: ";
    int id;
    leerEntero(id);
    Partido* p = buscarPartidoPorID(s, id);
    if (!p) {
        cout << "ERROR: Partido no encontrado." << endl;
        return;
    }
    if (strcmp(p->estado, "JUGADO") == 0) {
        cout << "ERROR: El partido ID " << id << " ya tiene resultado registrado." << endl;
        return;
    }
    
    int pL, pV;
    cout << "Puntos Local: ";
    leerEntero(pL);
    cout << "Puntos Visitante: ";
    leerEntero(pV);
    
    if (!confirmarAccion("Asentar marcador deportivo oficial?")) return;
    
    if (registrarResultado(s, id, pL, pV)) cout << "Estadisticas y marcador actualizados de manera automatica." << endl;
}

void menuBuscarPartido(SistemaDeportivo* s) {
    cout << "=== BUSCAR PARTIDO ===" << endl;
    cout << "Ingrese ID del partido: ";
    int id;
    leerEntero(id);
    Partido* p = buscarPartidoPorID(s, id);
    if (p) mostrarPartido(p, s);
    else cout << "ERROR: Inexistente." << endl;
}

void menuListarPartidos(SistemaDeportivo* s) {
    cout << "=== LISTADOS DE PARTIDOS ===" << endl;
    cout << "1. Todos" << endl;
    cout << "2. Por Estado (PROGRAMADO, JUGADO, CANCELADO)" << endl;
    int op;
    leerEntero(op);
    int cant = 0;
    if (op == 1) {
        Partido** res = listarPartidos(s, &cant);
        if (res) {
            mostrarListaPartidos(res, cant, s);
            delete[] res;
        } else cout << "Vacio." << endl;
    } else if (op == 2) {
        char est[20];
        cout << "Ingrese Estado exacto: ";
        cin.getline(est, 19);
        Partido** res = listarPartidosPorEstado(s, est, &cant);
        if (res) {
            mostrarListaPartidos(res, cant, s);
            delete[] res;
        } else cout << "Sin coincidencias." << endl;
    }
}

void menuCancelarPartido(SistemaDeportivo* s) {
    cout << "=== CANCELAR PARTIDO ===" << endl;
    cout << "ID del partido: ";
    int id;
    leerEntero(id);
    
    Partido* p = buscarPartidoPorID(s, id);
    if (!p) {
        cout << "ERROR: Id invalido." << endl;
        return;
    }
    if (strcmp(p->estado, "JUGADO") == 0) {
        cout << "ADVERTENCIA: El partido ya fue ejecutado. Cancelarlo revertira los puntos de los equipos." << endl;
    }
    if (!confirmarAccion("Proceder con la cancelacion irreversible?")) return;
    
    if (cancelarPartido(s, id)) cout << "Efectuado con exito." << endl;
    else cout << "ERROR: El partido no existe o ya se encuentra cancelado." << endl;
}

void configurarIdioma() {
    try {
        std::locale::global(std::locale(""));
        std::cout.imbue(std::locale());
    } catch (const std::exception& e) {
        std::locale::global(std::locale("C"));
        std::cout.imbue(std::locale());
    }

    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    #endif
}

// ============================================================================
// 9. NUCLEO Y FLUJO DE MENUS (MAIN)
// ============================================================================

int main() {

    configurarIdioma();

    SistemaDeportivo sistema;
    Torneo conf = {"Liga Apertura 2026", "Futbol", "GRUPOS", "2026-05-01", "2026-08-30"};
    inicializarSistema(&sistema, conf);
    
    int opPrincipal = -1;
    do {
        cout << "\n╔═══════════════════════════════════════════╗" << endl;
        cout << "║    SISTEMA DE GESTION DE TORNEOS          ║" << endl;
        cout << "║    Torneo: " << sistema.torneo.nombre << "             ║" << endl;
        cout << "║    Deporte: " << sistema.torneo.deporte << " | Formato: " << sistema.torneo.formato << "      ║" << endl;
        cout << "╠═══════════════════════════════════════════╣" << endl;
        cout << "║  1. Gestion de Equipos                    ║" << endl;
        cout << "║  2. Gestion de Jugadores                  ║" << endl;
        cout << "║  3. Gestion de Partidos                   ║" << endl;
        cout << "║  4. Tabla de Posiciones                   ║" << endl;
        cout << "║  0. Salir                                 ║" << endl;
        cout << "╚═══════════════════════════════════════════╝" << endl;
        cout << "Seleccione una opcion: ";
        leerEntero(opPrincipal);
        
        if (opPrincipal == 1) {
            int opSub = -1;
            do {
                cout << "\n╔═══════════════════════════════════════════╗\n";
                cout << "║          GESTION DE EQUIPOS               ║\n";
                cout << "╠═══════════════════════════════════════════╣\n";
                cout << "║  1. Registrar equipo                      ║\n";
                cout << "║  2. Buscar equipo                         ║\n";
                cout << "║  3. Actualizar equipo                     ║\n";
                cout << "║  4. Listar equipos                        ║\n";
                cout << "║  5. Eliminar equipo                       ║\n";
                cout << "║  0. Volver al menu principal              ║\n";
                cout << "╚═══════════════════════════════════════════╝\n";
                cout << "Seleccione una opcion: ";
                leerEntero(opSub);
                if (opSub == 1) menuRegistrarEquipo(&sistema);
                else if (opSub == 2) menuBuscarEquipo(&sistema);
                else if (opSub == 3) menuActualizarEquipo(&sistema);
                else if (opSub == 4) menuListarEquipos(&sistema);
                else if (opSub == 5) menuEliminarEquipo(&sistema);
            } while (opSub != 0);
            
        } else if (opPrincipal == 2) {
            int opSub = -1;
            do {
                cout << "\n╔═══════════════════════════════════════════╗\n";
                cout << "║        GESTION DE JUGADORES               ║\n";
                cout << "╠═══════════════════════════════════════════╣\n";
                cout << "║  1. Registrar jugador                     ║\n";
                cout << "║  2. Buscar jugador                        ║\n";
                cout << "║  3. Actualizar jugador                    ║\n";
                cout << "║  4. Listar jugadores (todos)              ║\n";
                cout << "║  5. Listar jugadores por equipo           ║\n";
                cout << "║  6. Eliminar jugador                      ║\n";
                cout << "║  0. Volver al menu principal              ║\n";
                cout << "╚═══════════════════════════════════════════╝\n";
                cout << "Seleccione una opcion: ";
                leerEntero(opSub);
                if (opSub == 1) menuRegistrarJugador(&sistema);
                else if (opSub == 2) menuBuscarJugador(&sistema);
                else if (opSub == 3) menuActualizarJugador(&sistema);
                else if (opSub == 4 || opSub == 5) menuListarJugadores(&sistema);
                else if (opSub == 6) menuEliminarJugador(&sistema);
            } while (opSub != 0);
            
        } else if (opPrincipal == 3) {
            int opSub = -1;
            do {
                cout << "\n╔═══════════════════════════════════════════╗\n";
                cout << "║        GESTION DE PARTIDOS                ║\n";
                cout << "╠═══════════════════════════════════════════╣\n";
                cout << "║  1. Programar partido                     ║\n";
                cout << "║  2. Registrar resultado                   ║\n";
                cout << "║  3. Buscar partido                        ║\n";
                cout << "║  4. Listar partidos                       ║\n";
                cout << "║  5. Cancelar partido                      ║\n";
                cout << "║  0. Volver al menu principal              ║\n";
                cout << "╚═══════════════════════════════════════════╝\n";
                cout << "Seleccione una opcion: ";
                leerEntero(opSub);
                if (opSub == 1) menuProgramarPartido(&sistema);
                else if (opSub == 2) menuRegistrarResultado(&sistema);
                else if (opSub == 3) menuBuscarPartido(&sistema);
                else if (opSub == 4) menuListarPartidos(&sistema);
                else if (opSub == 5) menuCancelarPartido(&sistema);
            } while (opSub != 0);
            
        } else if (opPrincipal == 4) {
            mostrarTablaPosiciones(&sistema);
        } else if (opPrincipal == 0) {
            if (!confirmarAccion("Seguro que desea salir de la aplicacion?")) {
                opPrincipal = -1; 
            }
        }
    } while (opPrincipal != 0);
    
    liberarSistema(&sistema);
    cout << "Memoria dinamica liberada correctamente conforme al heap. Ejecucion finalizada." << endl;
    return 0;
}