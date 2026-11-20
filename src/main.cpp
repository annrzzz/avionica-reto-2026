
#include <Arduino.h>

const int PIN_BOTON = 15;
const int LED_IDLE = 9;
const int LED_ARMED = 10;
const int LED_DEPLOYMENT = 11;
const int LED_LANDED = 12;
const int LED_IGNICION = 13;

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
                //ignorar
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
                //ignorar
            }
            break;
        case ASCENT:
            if (pulso == PULSO_CORTO)
            {
                estadoActual = DEPLOYMENT;
            }
            else 
            {
                //ignorar
            }
            break;
        case DEPLOYMENT:
            if (pulso == PULSO_CORTO)
            {
                estadoActual = LANDED;
            }
            else 
            {
                //ignprar
            }
            break;
        case LANDED:
            //ignorar
            break;
    }
}

void actualizarLEDs()
{
    digitalWrite(LED_IDLE, estadoActual == IDLE);
    digitalWrite(LED_ARMED, estadoActual == ARMED);
    digitalWrite(LED_DEPLOYMENT, estadoActual == DEPLOYMENT);
    digitalWrite(LED_LANDED, estadoActual == LANDED);
    digitalWrite(LED_IGNICION, LOW);
}

void loop() 
{
  Evento pulso = leerBoton();      
  if (pulso != NINGUNO) 
  {
        procesarEvento(pulso);
  } 
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
}
   