#include <Arduino.h>
#include "BasicStepperDriver.h" // includerea librării corespunzatoare utilizării și programării unui motor stepper
//majoritatea motoarelor de tip stepper realizează o mișcare de 1.8 grade/pas

//configurarea parametriilor motorului
#define MOTOR_STEPS 200           //numărul de pași corespunzători unei rotiri complete de 360 de grade
#define RPM 1                   //viteaza motorului = 1 rotație pe minut
#define MICROSTEPS 16             //reprezintă numărul de micro-pași pe pas complet
#define MS_nr 4                   //utilizat pentru a calcula numărul de pași
#define MS_grd 0.25               //utilizat pentru a calcula gradul de rotație
int step_nr=0;                    //variabilă de calcul pentru numărul dorit de grade
// sunt montati 3 jumperi sub modulul driver pentru a genera 16 microsteps

//inițializarea stărilor butoanelor și sistemului
int stare = 0; // inițializarea stării de comandă pe 0 - starea de așteptare comandă;
int stare_bhalt = 0; // initializarea stării butonului de oprire pe 0;
int stare_panica=0; // inițializarea stării de panică pe 0(neactivată);
int stare_b1=0; // inițializarea stării butonului 1 pe 0;
int stare_b2=0; // inițializarea stării butonului 2 pe 0;
int stare_b3=0; // inițializarea stării butonului 3 pe 0;
int stare_b4=0; // inițializarea stării butonului 4 pe 0;
int stare_brst=0; //inițializarea stării butonului de resetare pe 0(neapăsat);

//definirea pinilor pentru direcție, pași și butoane
#define DIR 5 //funcția de direcție a motorului
#define STEP 2 //funcția de pas a motorului
#define ENABLE 8 //funcția de activare a motorului
#define b1 3 // butonul pt programul 1 este conectat la pinul 3;
#define b2 4 // butonul pt programul 2 este conectat la pinul 4;
#define b3 6 // butonul pt programul 3 este conectat la pinul 6;
#define b4 7 // butonul pt programul 4 este conectat la pinul 7;
#define bhalt A2 // butonul de oprire este conectat la pinul A2;
#define brst A3 // butonul de reset software este conectat la pinul A3;
#define blims A0 // butonul de limită este conectat la pinul A0;
#define LP1 10 //led program 1;
#define LP2 11 //led program 2;
#define LP3 12 //led program 3;
#define LP4 9 //led program 4;
#define LS 13 //led buton oprire;

BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP, ENABLE);

//configurarea inițială a motorului și pinilor
void setup() {
    stepper.begin(RPM, MICROSTEPS);
    stepper.setEnableActiveState(HIGH);
    pinMode(b1, INPUT_PULLUP);
    pinMode(b2, INPUT_PULLUP);
    pinMode(b3, INPUT_PULLUP);
    pinMode(b4, INPUT_PULLUP);
    pinMode(bhalt, INPUT_PULLUP);
    pinMode(brst, INPUT_PULLUP);
    pinMode(blims, INPUT_PULLUP);
    //am utilizat INPUT_PULLUP pentru a accesa rezistențele interne ale plăcuței 
    pinMode(LP1, OUTPUT);
    pinMode(LP2, OUTPUT);
    pinMode(LP3, OUTPUT);
    pinMode(LP4, OUTPUT);
    pinMode(LS, OUTPUT);
    
    stare_brst=0;
    stepper.enable();
    
    //procedura calibrare la pornirea aparatului
    stare_brst=1; //setarea stării de rest pe 1 (faza de resetare activată)
    while(stare_brst==1)  //se verfică a fost sau nu apăsat limitatorul
    { 
      if(digitalRead(blims) == LOW){
        stare_brst=0; //resetarea stării de rest pe 0 (faza de resetare neactivată)
      } 
      else{
        stepper.rotate(-MS_grd); //rotirea motorului cu un pas
      }
    }
    step_nr=100*MS_nr;
        for(int i=1;i<=step_nr;i++)
          {stepper.rotate(MS_grd);} //rotirea motorului până la poziția de pornire dorită
          
    //închiderea procedurii de calibrare
}
void loop() {
  //se verifică dacă a fost apăsat butonul de oprire
  if(analogRead(bhalt)<100) {
    opresteSistemul();
  }
  //se verifică dacă a fost apăsat butonul de reset
  if(analogRead(brst)<100){
    stare_panica=0; 
    stare_brst=1;
    stepper.enable();
    // procedura de reset poziție
    while(stare_brst==1) //se verfică a fost sau nu apăsat limitatorul
    { 
      if(digitalRead(blims) == LOW){
        stare_brst=0; //resetarea stării de rest pe 0 (faza de resetare neactivată)
      }
      else{
        stepper.rotate(-MS_grd);  //rotirea motorului cu un pas
      }
    }
    step_nr=100*MS_nr;
        for(int i=1;i<=step_nr;i++)
          {stepper.rotate(MS_grd);}  //rotirea motorului până la poziția de pornire dorită
   
    digitalWrite(LS,LOW);
    stare_brst=0;
  }
  
  // se EXECUTĂ programul CÂT TIMP butonul de oprire NU a fost apăsat
  while(stare_panica==0) 
  { 
    stare=0;  // inițializarea stări aparatului cu 0, corespunzătoare stării de așteptare comandă
    
    if (analogRead(bhalt) < 100) // dacă butonul de oprire a fost apăsat, atunci se parasește bucla de acționare a echipamentului
    { opresteSistemul();
      return;  // se iese din buclă
    }

    // CITIREA STARILOR BUTOANELOR 1,2,3,4
    stare_b1 = digitalRead(b1); // se verifică starea butonului pentru programul 1
    stare_b2 = digitalRead(b2); // se verifică starea butonului pentru programul 2
    stare_b3 = digitalRead(b3); // se verifică starea butonului pentru programul 3
    stare_b4 = digitalRead(b4); // se verifică starea butonului pentru programul 4


   //ATRIBUIRE VALORI STARE ÎN FUNCȚIE DE PROGRAMUL SELECTAT
   if (stare==0 and stare_b1 == 0) // se verfică dacă butonul pentru programul 1 a fost apăsat și dacă nu s-a apăsat butonul de oprire
      {
        stare=1; // setarea stării echipamentului cu 1, corespunzătoare programului 1
      }
   if (stare==0 and stare_b2 == 0) // se verfică dacă butonul pentru programul 2 a fost apăsat și dacă nu s-a apăsat butonul de oprire
      {
        stare=2; // setarea stării echipamentului cu 2, corespunzătoare programului 2
      } 
   if (stare==0 and stare_b3 == 0) // se verfică dacă butonul pentru programul 3 a fost apăsat și dacă nu s-a apăsat butonul de oprire
      {
        stare=3; // setarea stării echipamentului cu 3, corespunzătoare programului 3
      } 
   if (stare==0 and stare_b4 == 0) // se verfică dacă butonul pentru programul 4 a fost apăsat și dacă nu s-a apăsat butonul de oprire
      {
        stare=4; // setarea stării echipamentului cu 4, corespunzătoare programului 4
      } 


   //EXECUȚIE PROGRAME
    // execuția programului 1 - flexie 30 grade / extensie 30 grade
      if (stare ==1){
        stepper.enable();
        digitalWrite(LP1, HIGH); //aprinderea led-ului butonului pentru programul 1
        //execuția de 3 ori a mișcării
        for (int j = 0; j < 3; j++) {
        step_nr=30*MS_nr;
        for (int i=1; i<= step_nr; i++) // motorul se rotește cu 30 de grade în sus
        {
          if (analogRead(bhalt) < 100) //verificare dacă s-a apăsat butonul de oprire
          { opresteSistemul();
            return; } // ieșirea din program dacă s-a apăsat butonul de oprire
          else { stepper.rotate(MS_grd); } //direcția morotului - în sus cu un grad
        }
        step_nr=60*MS_nr;
        for (int i=1; i<= step_nr; i++) // motorul se rotește cu 60 de grade în jos
        {
          if (analogRead(bhalt) < 100)//verificare dacă s-a apăsat butonul de oprire
          { opresteSistemul();
            return; } // ieșirea din program dacă s-a apăsat butonul de oprire
          else { stepper.rotate(-MS_grd); } //direcția morotului - în jos cu un grad
        }
        step_nr=30*MS_nr;
        for (int i=1; i<= step_nr; i++) // motorul se rotește cu 30 de grade în sus
        {
          if (analogRead(bhalt) < 100) //verificare dacă s-a apăsat butonul de oprire
          { opresteSistemul();
            return; } // ieșirea din program dacă s-a apăsat butonul de oprire
          else { stepper.rotate(MS_grd); } //direcția morotului - în sus cu un grad
        }

        }
        //stepper.disable();
        digitalWrite(LP1, LOW);
      }


    // executia programului 2 - flexie 50 grade/ extensie 50 de grade
     if (stare ==2){
        stepper.enable();
        digitalWrite(LP2, HIGH); //aprinderea led-ului butonului pentru programul 2
        //execuția de 3 ori a mișcării
        for (int j = 0; j < 3; j++) {
        step_nr=50*MS_nr;
        for (int i=1; i<= step_nr; i++) // motorul se roteste cu 50 de grade în sus
        {
          if (analogRead(bhalt) < 100) //verificare dacă s-a apăsat butonul de oprire
          { opresteSistemul();
            return; } // ieșirea din program dacă s-a apăsat butonul de oprire
          else { stepper.rotate(MS_grd); } //direcția morotului - în sus cu un grad
        }
        step_nr=100*MS_nr;
        for (int i=1; i<= step_nr; i++) // motorul se roteste cu 100 de grade în jos
        {
          if (analogRead(bhalt) < 100)//verificare dacă s-a apăsat butonul de oprire
          { opresteSistemul();
            return; } // ieșirea din program dacă s-a apăsat butonul de oprire
          else { stepper.rotate(-MS_grd); } //direcția morotului - în jos cu un grad
        }
        step_nr=50*MS_nr;
        for (int i=1; i<= step_nr; i++) // motorul se roteste cu 50 de grade în sus
        {
          if (analogRead(bhalt) < 100) //verificare dacă s-a apăsat butonul de oprire
          { opresteSistemul();
            return; } // ieșirea din program dacă s-a apăsat butonul de oprire
          else { stepper.rotate(MS_grd); } //direcția morotului - în sus cu un grad
        }
        }
       // stepper.disable();
        digitalWrite(LP2, LOW);
      }
  
  
    // executia programului 3 - deviatie radiala 10 grade / deviatie ulnara 25 grade
      if (stare == 3){
        stepper.enable();
        digitalWrite(LP3, HIGH); //aprinderea led-ului butonului pentru programul 3
        //execuția de 3 ori a mișcării
        for (int j = 0; j < 3; j++) {
        step_nr=10*MS_nr;
        for (int i=1; i<= step_nr; i++) // motorul se roteste cu 10 de grade în sus
        {
          if (analogRead(bhalt) < 100) //verificare dacă s-a apăsat butonul de oprire
          { opresteSistemul();
            return; } // ieșirea din program dacă s-a apăsat butonul de oprire
          else { stepper.rotate(MS_grd); } //direcția morotului - în sus cu un grad
        }
        step_nr=35*MS_nr;
        for (int i=1; i<= step_nr; i++) // motorul se roteste cu 35 de grade în jos
        {
          if (analogRead(bhalt) < 100)//verificare dacă s-a apăsat butonul de oprire
          { opresteSistemul();
            return; } // ieșirea din program dacă s-a apăsat butonul de oprire
          else { stepper.rotate(-MS_grd); } //direcția morotului - în jos cu un grad
        }
        step_nr=25*MS_nr;
        for (int i=1; i<= step_nr; i++) // motorul se roteste cu 25 de grade în sus
        {
          if (analogRead(bhalt) < 100) //verificare dacă s-a apăsat butonul de oprire
          { opresteSistemul();
            return; } // ieșirea din program dacă s-a apăsat butonul de oprire
          else { stepper.rotate(MS_grd); } //direcția morotului - în sus cu un grad
        }
        }
       // stepper.disable();
        digitalWrite(LP3, LOW);
      }
  
  
    // executia programului 4 - deviatie radiala 15 grade / deviatie ulnara 30 grade
      if (stare ==4){
        stepper.enable();
        digitalWrite(LP4, HIGH); //aprinderea led-ului butonului pentru programul 4
        //execuția de 3 ori a mișcării
        for (int j = 0; j < 3; j++) {
        step_nr=15*MS_nr;
        for (int i=1; i<= step_nr; i++) // motorul se roteste cu 15 de grade în sus
        {
          if (analogRead(bhalt) < 100) //verificare dacă s-a apăsat butonul de oprire
          { opresteSistemul();
            return; } // ieșirea din program dacă s-a apăsat butonul de oprire
          else { stepper.rotate(MS_grd); } //direcția morotului - în sus cu un grad
        }
        step_nr=45*MS_nr;
        for (int i=1; i<= step_nr; i++) // motorul se roteste cu 45 de grade în jos
        {
          if (analogRead(bhalt) < 100)//verificare dacă s-a apăsat butonul de oprire
          { opresteSistemul();
            return; } // ieșirea din program dacă s-a apăsat butonul de oprire
          else { stepper.rotate(-MS_grd); } //direcția morotului - în jos cu un grad
        }
        step_nr=30*MS_nr;
        for (int i=1; i<= step_nr; i++) // motorul se roteste cu 30 de grade în sus
        {
          if (analogRead(bhalt) < 100) //verificare dacă s-a apăsat butonul de oprire
          { opresteSistemul();
            return; } // ieșirea din program dacă s-a apăsat butonul de oprire
          else { stepper.rotate(MS_grd); } //direcția morotului - în sus cu un grad
        }
        }
        //  stepper.disable();
        digitalWrite(LP4, LOW);
      }

    }
}

void opresteSistemul() {
  stepper.disable();
  digitalWrite(LS, HIGH); // Aprinde LED-ul butonului de oprire
  digitalWrite(LP1, LOW);  // Stinge LED 1
  digitalWrite(LP2, LOW);  // Stinge LED 2
  digitalWrite(LP3, LOW);  // Stinge LED 3
  digitalWrite(LP4, LOW);  // Stinge LED 4
  stare_panica=1;
  return; // Încheie funcția
}
