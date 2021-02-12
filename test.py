from machine import Pin, PWM
from utime import sleep, sleep_ms, ticks_ms, ticks_diff
from urandom import randrange as rand

light_pin_numbers = [13, 12, 11, 10]
button_pin_numbers = [18, 19, 20, 21]
buzzer_pin_number = 9

assert len(light_pin_numbers) == len(button_pin_numbers)


class ButtonHelper:
    def __init__(self, pin_number, pull=Pin.PULL_DOWN):
        self.pin = Pin(pin_number, Pin.IN, pull)
        self.last_state = self.pin.value()
        self.last_flicker = self.last_state
        self.timestamp = ticks_ms()

    def change_to(self, debounce_time=20):
        current_state = self.pin.value()

        if current_state == self.last_state:
            return None

        if current_state != self.last_flicker:
            self.timestamp = ticks_ms()
            self.last_flicker = current_state

        if ticks_diff(ticks_ms(), self.timestamp) > debounce_time:
            self.last_state = current_state
            return self.last_state
        return None


class BuzzerHelper:
    NOTES = [440, 330, 277, 165]
    DUTY = 1000

    def __init__(self, pin_number):
        self.buzzer = PWM(Pin(pin_number, Pin.OUT))

    def play_color(self, c):
        self.buzzer.freq(self.NOTES[c % len(self.NOTES)])
        self.buzzer.duty_u16(self.DUTY)

    def play_win(self):
        self.buzzer.freq(660)
        self.buzzer.duty_u16(self.DUTY)
        sleep_ms(150)
        self.buzzer.duty_u16(0)
        sleep_ms(20)
        self.buzzer.freq(880)
        self.buzzer.duty_u16(self.DUTY)
        sleep_ms(400)
        self.buzzer.duty_u16(0)

    def play_loose(self):
        self.buzzer.freq(180)
        self.buzzer.duty_u16(self.DUTY)
        sleep_ms(300)
        self.buzzer.duty_u16(0)
        sleep_ms(20)
        self.buzzer.freq(138)
        self.buzzer.duty_u16(self.DUTY)
        sleep_ms(300)
        self.buzzer.freq(110)
        self.buzzer.duty_u16(self.DUTY)
        sleep_ms(1200)
        self.buzzer.duty_u16(0)

    def play_ready(self):
        self.buzzer.freq(1760)
        self.buzzer.duty_u16(self.DUTY)
        sleep_ms(100)
        self.buzzer.duty_u16(0)


    def stop(self):
        self.buzzer.duty_u16(0)

lights = [Pin(l, Pin.OUT) for l in light_pin_numbers]
buttons = [ButtonHelper(b) for b in button_pin_numbers]
buzzer = BuzzerHelper(6)


def start_sequence():
    for _ in range(5):
        for l in lights:
            l.on()
            sleep(0.1)
            l.off()

def wait_button_push():
    lights_pwm = [PWM(p) for p in lights]

    intensity = 0
    direction = 1
    pressed = False
    while not pressed:
        duty = intensity * intensity
        for l in lights_pwm:
            l.duty_u16(duty)
        sleep_ms(1)

        if intensity == 255:
            direction = -1
        elif intensity == 0:
            direction = 1
        intensity += direction


        for b in buttons:
            if b.change_to() == 1:
                pressed = True
                break

    for l, p in zip(lights, lights_pwm):
        l.init(mode=Pin.OUT, value=0)
        p.deinit()


class SimonGame():
    def __init__(self, lights, buttons, buzzer):
        self.seq = []
        self.lights = lights
        self.buttons = buttons
        self.buzzer = buzzer

    def start(self):
        while True:
            self.next_round()
            self.display_seq()
            buzzer.play_ready()
            win = self.guess_round()
            sleep(0.3)
            if win:
                buzzer.play_win()
                sleep(2)
            else:
                buzzer.play_loose()
                break

    def next_round(self):
        self.seq.append(rand(len(self.lights)))


    def display_seq(self):
        for l in self.lights:
            l.off()
        for c in self.seq:
            self.lights[c].on()
            self.buzzer.play_color(c)
            sleep(0.1)
            self.buzzer.stop()
            sleep(0.2)
            self.lights[c].off()
            sleep(0.8)

    def guess_round(self):
        for color in self.seq:
            guessed = False
            while not guessed:
                for c, b in enumerate(buttons):
                    if b.change_to() == 1:
                        if c == color:
                            self.buzzer.play_color(c)
                            self.lights[c].on()
                            while b.change_to() != 0:
                                pass
                            self.buzzer.stop()
                            self.lights[c].off()
                            guessed = True
                        else:
                            return False
        return True

start_sequence()
sleep(0.3)
while True:
    wait_button_push()
    sleep(2)
    game = SimonGame(lights, buttons, buzzer)
    game.start()
