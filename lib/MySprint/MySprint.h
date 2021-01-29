#ifndef MYPRINT_H
#define MYPRINT_H

#if DEBUG == 1
#define SprintBegin(a) Serial.begin(a)
#define Sprint(a) (Serial.print(a))
#define Sprintln(a) (Serial.println(a))
#define Sprintf(a, b) (Serial.printf(a, b))
#else
#define SprintBegin(a)
#define Sprint(a)
#define Sprintln(a)
#define Sprintf(a, b)
#endif

#endif //MYPRINT_H