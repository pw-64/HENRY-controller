# HENRY-controller

An arduino program to control solenoid valves and vacuum pumps on a microscope sample processing machine. It uses an E-Ink display, temperature sensors, vacuum / pressure sensors, relays and buttons.

When turned on, it presents messages to the user in the top area, and button choices in the lower area. After choosing a function, the arduino controls the physical machinery such as firing relays. Throughout the process, messages are pushed to the screen to indicate the current status. Upon completion, the controller will pick different functions to be available.

The process is:
```mermaid
graph
off[Power Off] -- "Turn On (Power Connected / Reset Pin)" ---> Initialisation
Initialisation --> Pump & Vent
Pump <---> Vent
Pump --> Heat --> Vent
Pump -- Turn Off --> pumps_off[Scroll Pump Off & Turbo Spindown] --> Pump & Vent
```

# Pictures
![Filling In Unused Button](https://github.com/pw-64/HENRY-controller/blob/main/Photos/After%20Filling%20In%20Unused%20Button.jpg)
![After Pumping](https://github.com/pw-64/HENRY-controller/blob/main/Photos/After%20Pumping.jpg)
![Breadboard 1](https://github.com/pw-64/HENRY-controller/blob/main/Photos/Breadboard%201.jpg)
![Breadboard 2](https://github.com/pw-64/HENRY-controller/blob/main/Photos/Breadboard%202.jpg)
![Front Panel Testing](https://github.com/pw-64/HENRY-controller/blob/main/Photos/Front%20Panel%20Testing.jpg)
![Inside The HENRY](https://github.com/pw-64/HENRY-controller/blob/main/Photos/Inside%20The%20HENRY.jpg)
![Pump Example](https://github.com/pw-64/HENRY-controller/blob/main/Photos/Pump%20Example.jpg)
![Turbo Spindown 1](https://github.com/pw-64/HENRY-controller/blob/main/Photos/Turbo%20Spindown%201.jpg)
![Turbo Spindown 2](https://github.com/pw-64/HENRY-controller/blob/main/Photos/Turbo%20Spindown%202.jpeg)
![Wiring 1](https://github.com/pw-64/HENRY-controller/blob/main/Photos/Wiring%201.jpg)
![Wiring 2](https://github.com/pw-64/HENRY-controller/blob/main/Photos/Wiring%202.jpg)
![After Pumping](https://github.com/pw-64/HENRY-controller/blob/main/Photos/After%20Pumping.jpg)
![Breadboard 1](https://github.com/pw-64/HENRY-controller/blob/main/Photos/Breadboard%201.jpg)
![Breadboard 2](https://github.com/pw-64/HENRY-controller/blob/main/Photos/Breadboard%202.jpg)
![Front Panel Testing](https://github.com/pw-64/HENRY-controller/blob/main/Photos/Front%20Panel%20Testing.jpg)
![Inside The HENRY](https://github.com/pw-64/HENRY-controller/blob/main/Photos/Inside%20The%20HENRY.jpg)
![Pump Example](https://github.com/pw-64/HENRY-controller/blob/main/Photos/Pump%20Example.jpg)
![Turbo Spindown 1](https://github.com/pw-64/HENRY-controller/blob/main/Photos/Turbo%20Spindown%201.jpg)
![Turbo Spindown 2](https://github.com/pw-64/HENRY-controller/blob/main/Photos/Turbo%20Spindown%202.jpeg)
![Wiring 1](https://github.com/pw-64/HENRY-controller/blob/main/Photos/Wiring%201.jpg)
![Wiring 2](https://github.com/pw-64/HENRY-controller/blob/main/Photos/Wiring%202.jpg)
