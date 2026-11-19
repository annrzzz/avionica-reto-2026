# avionica-reto-2026
Proyecto avionica para AAIAQ

pines boton-15/morado-10/verde-11/azul-12/rojo-13

#include <Arduino.h> //cpp

const int PIN_BOTON = 15;
const int LED_IDLE
const int LED_ARMED
const int LED_DEPLOYMENT
const int LED_IGNICION = 13;

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
}

Estado estadoActual = IDLE;