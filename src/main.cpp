
#include <Arduino.h>

const int PIN_BOTON = 15;
const int LED_IDLE = 9;
const int LED_ARMED = 10;
const int LED_DEPLOYMENT = 11;
const int LED_LANDED = 12;
const int LED_IGNICION = 13;
const int PIN_BOTON_TIERRA = 14;

bool tierraAntes = false;
unsigned long ultimoCambioTierra = 0;

const bool PRESIONADO = HIGH; //para que detecte si el boton cierra el circuito o no

enum Estado  
{
    IDLE, 
    ARMED, 
    ASCENT, 
    DEPLOYMENT, 
    LANDED
};

enum Evento
{
    NINGUNO,
    PULSO_CORTO,
    PULSO_LARGO,
    PULSO_RARO
};

Estado estadoActual = IDLE;


bool ignicionHecha = false;          // para que solo ocurra una vez
bool ignicionActiva = false;         // ¿el pulso está en curso?
unsigned long inicioIgnicion = 0;    // cuándo empezó el pulso

unsigned long inicioAscent = 0;     // cuándo entró a ASCENT
bool ascentRegistrado = false;      // para guardar la hora solo una vez

bool botonAntes = false;          // ¿estaba presionado en la vuelta anterior?
unsigned long inicioPulso = 0;    // cuándo empezó la pulsación
unsigned long ultimoCambio = 0;   // para el antirrebote

// Detectar cuando el boton se presiona y guardar millis() como inicio para luego detectar cuando se suelta y calcular la duracion que serian los millis menos el inicio. Si la duracion < 1000 ms --PULSO_CORTO, si la duracion >= 2000 ms --PULSO_LARGO, si esta entre 1000 y 1999 ms --PULSO_RARO, si no pasa nada absoltmente nada --NINGUNO

Evento leerBoton()
{
    // antirrebote: si hubo un cambio hace menos de 30 ms, no hacemos nada
    if (millis() - ultimoCambio < 30)
    {
        return NINGUNO;
    }

    bool botonAhora = (digitalRead(PIN_BOTON) == PRESIONADO);

    if (botonAhora && !botonAntes)
    {
        // acaban de presionar
        inicioPulso = millis();
        botonAntes = true;
        ultimoCambio = millis();
    }
    else if (!botonAhora && botonAntes)
    {
        // acaban de soltar: ahora medimos cuánto duró
        unsigned long duracion = millis() - inicioPulso;
        botonAntes = false;
        ultimoCambio = millis();

        if (duracion < 1000)
        {
            return PULSO_CORTO;
        }
        else if (duracion >= 2000)
        {
            return PULSO_LARGO;
        }
        else
        {
            return PULSO_RARO;   // entre 1000 y 1999 ms
        }
    }

    return NINGUNO;
}

String nombreEstado(Estado estado)
{
    switch (estado)
    {
        case IDLE:
            return "IDLE";
        case ARMED:
            return "ARMED";
        case ASCENT:
            return "ASCENT";
        case DEPLOYMENT:
            return "DEPLOYMENT";
        case LANDED:
            return "LANDED";
    }
    return "DESCONOCIDO";
}

void cambiarEstado(Estado nuevo, String motivo)
{
    Serial.print("[");
    Serial.print(millis());
    Serial.print(" ms] ");
    Serial.print(nombreEstado(estadoActual));
    Serial.print(" -> ");
    Serial.print(nombreEstado(nuevo));
    Serial.print(" (");
    Serial.print(motivo);
    Serial.println(")");

    estadoActual = nuevo;
}

void registrarIgnorado()
{
    Serial.print("[");
    Serial.print(millis());
    Serial.print(" ms] Ignorado en ");
    Serial.println(nombreEstado(estadoActual));
}

void procesarEvento(Evento pulso)
{
    switch (estadoActual) 
    {
        case IDLE:
            if (pulso == PULSO_LARGO)
            {
                cambiarEstado(ARMED, "long press");
            }
            else 
            {
                registrarIgnorado();
            }
            break;
        case ARMED:
            if (pulso == PULSO_LARGO)
            {
                cambiarEstado(IDLE, "long press");
            }
            else if (pulso == PULSO_CORTO)
            {
                cambiarEstado(ASCENT, "short press");
            }
            else
            {
                registrarIgnorado();
            }
            break;
        case ASCENT:
            if (pulso == PULSO_CORTO)
            {
                cambiarEstado(DEPLOYMENT, "short press");
            }
            else 
            {
                registrarIgnorado();
            }
            break;
        case DEPLOYMENT:
            if (pulso == PULSO_CORTO)
            {
                cambiarEstado(LANDED, "short press");
            }
            else 
            {
                registrarIgnorado();
            }
            break;
        case LANDED:
            registrarIgnorado();
            break;
    }
}

void actualizarLEDs()
{
    // todos apagados por defecto
    digitalWrite(LED_IDLE, LOW);
    digitalWrite(LED_ARMED, LOW);
    digitalWrite(LED_DEPLOYMENT, LOW);
    digitalWrite(LED_LANDED, LOW);
    digitalWrite(LED_IGNICION, ignicionActiva);

    if (estadoActual == IDLE)
    {
        digitalWrite(LED_IDLE, HIGH);            // se queda fijo
    }
    else if (estadoActual == ARMED)
    {
        if ((millis() / 500) % 2 == 0)           // va lento
        {
            digitalWrite(LED_ARMED, HIGH);
        }
    }
    else if (estadoActual == ASCENT)
    {
        if ((millis() / 150) % 2 == 0)           // va rápido
        {
            digitalWrite(LED_ARMED, HIGH);
        }
    }
    else if (estadoActual == DEPLOYMENT)
    {
        digitalWrite(LED_DEPLOYMENT, HIGH);      // igual se queda fijo
    }
    else if (estadoActual == LANDED)
    {
        digitalWrite(LED_LANDED, HIGH);          // se queda fijp
    }
}

void actualizarIgnicion()
{
    // al entrar a DEPLOYMENT, arranca el pulso (solo la primera vez)
    if (estadoActual == DEPLOYMENT && !ignicionHecha)
    {
        ignicionHecha = true;
        ignicionActiva = true;
        inicioIgnicion = millis();
    }

    // cuando pasan 1000 ms, se apaga
    if (ignicionActiva && millis() - inicioIgnicion >= 1000)
    {
        ignicionActiva = false;
    }
}

void leerBotonTierra()
{
    // antirrebote
    if (millis() - ultimoCambioTierra < 30)
    {
        return;
    }

    bool tierraAhora = (digitalRead(PIN_BOTON_TIERRA) == PRESIONADO);

    if (tierraAhora != tierraAntes)
    {
        tierraAntes = tierraAhora;
        ultimoCambioTierra = millis();

        // solo cuando se PRESIONA, y solo si estamos en vuelo
        if (tierraAhora && estadoActual == ASCENT)
        {
            cambiarEstado(DEPLOYMENT, "ground command");
        }
    }
}


void actualizarTemporizador()
{
    if (estadoActual == ASCENT)
    {
        // primera vuelta en ASCENT: guarda la hora de entrada
        if (!ascentRegistrado)
        {
            ascentRegistrado = true;
            inicioAscent = millis();
        }

        // pasaron 8 s sin pulsación: despliega
        if (millis() - inicioAscent >= 8000)
        {
            cambiarEstado(DEPLOYMENT, "8 s backup timer");
        }
    }
    else
    {
        ascentRegistrado = false;   // al salir de ASCENT, se reinicia
    }
}

void loop() 
{
  Evento pulso = leerBoton();      
  if (pulso != NINGUNO) 
  {
        procesarEvento(pulso);
  } 
  leerBotonTierra();
  actualizarTemporizador();
  actualizarIgnicion();
  actualizarLEDs();
}


void setup() 
{
  Serial.begin(115200);
  pinMode(PIN_BOTON, INPUT);
  pinMode(LED_IDLE, OUTPUT);
  pinMode(LED_ARMED, OUTPUT);
  pinMode(LED_DEPLOYMENT, OUTPUT);
  pinMode(LED_LANDED, OUTPUT);
  pinMode(LED_IGNICION, OUTPUT);
  pinMode(PIN_BOTON_TIERRA, INPUT);

  Serial.println("[0 ms] IDLE (reset)");
}
   
// Link aaaaa el wokwi de mi diseño de sistema de ingicion en fisico https://wokwi.com/projects/481073306276356097
