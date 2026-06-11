#include <iostream>
#include <cstring>
#include <limits>
#include <cstdlib>
#include <clocale>

using namespace std;


// 1. estructura del proyecto


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

struct SistemaDeportivo {
    Torneo torneo;              
    Equipo* equipos;
    int      numEquipos;
    int      capacidadEquipos;
    int siguienteIdEquipo;
};


// condiciones y datos


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


// 2. INICIALIZACIÓN Y LIBERACIÓN

void inicializarSistema(SistemaDeportivo* s, Torneo torneo) {
    s->torneo = torneo;
    s->capacidadEquipos = 4;
    s->numEquipos = 0;
    s->equipos = new Equipo[s->capacidadEquipos];
    s->siguienteIdEquipo = 1;
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

void liberarSistema(SistemaDeportivo* s) {
    if (s->equipos) {
        delete[] s->equipos;
        s->equipos = nullptr;
    }
    s->numEquipos = 0;
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


// 3. CAPA DE LÓGICA - EQUIPOS

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
    strncpy(nuevo.fechaRegistro, "2026-06-10", 10); 
    
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

// Declaración adelantada de funciones de presentación necesarias para los menús
void mostrarEquipo(Equipo* equipo);
void mostrarListaEquipos(Equipo** equipos, int cantidad);

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
    
    for (int i = index; i < s->numEquipos - 1; i++) {
        s->equipos[i] = s->equipos[i + 1];
    }
    s->numEquipos--;
    return true;
}


// 4. CAPA DE PRESENTACIÓN - EQUIPOS

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
    
    if (!confirmarAccion("Esta seguro de eliminar este equipo?")) return;
    
    if (eliminarEquipo(s, id)) {
        cout << "Equipo removido exitosamente." << endl;
    } else {
        cout << "ERROR: No se pudo eliminar el equipo." << endl;
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
    cout << "║                      " << s->torneo.nombre << "                               ║" << endl;
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


// 5. FUNCIÓN PRINCIPAL


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
        cout << "        GESTIÓN DE EQUIPOS - " << sistema.torneo.nombre << "\n";
        cout << "==================================================\n";
        cout << "1. Registrar Equipo\n";
        cout << "2. Buscar Equipo\n";
        cout << "3. Actualizar Equipo\n";
        cout << "4. Eliminar Equipo\n";
        cout << "5. Listar todos los Equipos\n";
        cout << "6. Ver Tabla de Posiciones\n";
        cout << "7. Salir del Programa\n";
        cout << "==================================================\n";
        cout << "Seleccione una opcion: ";
        leerEntero(opcion);

        system("cls");
        switch(opcion) {
            case 1: menuRegistrarEquipo(&sistema); break;
            case 2: menuBuscarEquipo(&sistema); break;
            case 3: menuActualizarEquipo(&sistema); break;
            case 4: menuEliminarEquipo(&sistema); break;
            case 5: menuListarEquipos(&sistema); break;
            case 6: mostrarTablaPosiciones(&sistema); break;
            case 7: cout << "Saliendo del modulo de equipos...\n"; break;
            default: cout << "Opcion invalida.\n"; break;
        }
        
        if (opcion != 7) {
            cout << "\nPresione Enter para continuar...";
            cin.get();
        }
        
    } while (opcion != 7);

    liberarSistema(&sistema);
    cout << "\nMemoria liberada correctamente. Programa cerrado.\n";
    return 0;
}