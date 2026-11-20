
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
Estado estadoAnterior = IDLE;

bool ignicionHecha = false;          // para que solo ocurra una vez
bool ignicionActiva = false;         // ¿el pulso está en curso?
unsigned long inicioIgnicion = 0;    // cuándo empezó el pulso

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

void registrarIgnorado()
{
    Serial.print(millis());
    Serial.print(" ms Ignorado en ");

    switch (estadoActual)
    {
        case IDLE:       
            Serial.println("IDLE");       
        break;
        case ARMED:      
            Serial.println("ARMED");      
        break;
        case ASCENT:     
            Serial.println("ASCENT");     
        break;
        case DEPLOYMENT: 
            Serial.println("DEPLOYMENT"); 
        break;
        case LANDED:     
            Serial.println("LANDED");     
        break;
    }
}

void procesarEvento(Evento pulso)
{
    switch (estadoActual) 
    {
        case IDLE:
            if (pulso == PULSO_LARGO)
            {
                estadoActual = ARMED;
            }
            else 
            {
                registrarIgnorado();
            }
            break;
        case ARMED:
            if (pulso == PULSO_LARGO)
            {
                estadoActual = IDLE;
            }
            else if (pulso == PULSO_CORTO)
            {
                estadoActual = ASCENT;
            }
            else
            {
                registrarIgnorado();
            }
            break;
        case ASCENT:
            if (pulso == PULSO_CORTO)
            {
                estadoActual = DEPLOYMENT;
            }
            else 
            {
                registrarIgnorado();
            }
            break;
        case DEPLOYMENT:
            if (pulso == PULSO_CORTO)
            {
                estadoActual = LANDED;
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
    digitalWrite(LED_IDLE, estadoActual == IDLE);
    digitalWrite(LED_ARMED, estadoActual == ARMED);
    digitalWrite(LED_DEPLOYMENT, estadoActual == DEPLOYMENT);
    digitalWrite(LED_LANDED, estadoActual == LANDED);
    digitalWrite(LED_IGNICION, ignicionActiva);
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
            estadoActual = DEPLOYMENT;
        }
    }
}

void imprimirEstado()
{
    if (estadoActual != estadoAnterior)
    {
        Serial.print(millis());
        Serial.print(" ms ");

        switch (estadoActual)
        {
            case IDLE:       
                Serial.println("IDLE");       
            break;
            case ARMED:      
                Serial.println("ARMED");      
            break;
            case ASCENT:     
                Serial.println("ASCENT");     
            break;
            case DEPLOYMENT: 
                Serial.println("DEPLOYMENT"); 
            break;
            case LANDED:     
                Serial.println("LANDED");     
            break;
        }

        estadoAnterior = estadoActual;
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
  actualizarIgnicion();
  actualizarLEDs();
  imprimirEstado();
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

  Serial.print(millis());
  Serial.println(" ms IDLE");
}
   
// Link aaaaa el wokwi de mi diseño de sistema de ingicion en fisico https://wokwi.com/projects/481073306276356097
