# avionica-reto-2026
Proyecto avionica para AAIAQ

pines boton-15/morado-9/verde-10/azul-11/naranja-12/rojo-13

#include <Arduino.h> //cpp

const int PIN_BOTON = 15;
const int LED_IDLE = 9;
const int LED_ARMED = 10;
const int LED_DEPLOYMENT = 11;
const int LED_LANDED = 12;
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
