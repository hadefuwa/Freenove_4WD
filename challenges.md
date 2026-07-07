# Freenove 4WD Car Challenges

Welcome! This is your mission book for the Freenove 4WD Car. Work through each lesson, try the challenges, and don't be afraid to break things — that's how you learn to code!

---

## Guide: How to Download Your Code

1. In the Arduino IDE, go to **Sketch -> Export Compiled Binary**.
2. Open your sketch folder (**Ctrl+K**). You will see a file ending in `.uf2`.
3. Drag and drop that `.uf2` file directly into the **RPI-RP2** drive.
4. The Pico will automatically reboot and start running your code.

**If you can't see the Pi on your PC then...**

1. Unplug the Pico from your computer.
2. Press and hold the **BOOTSEL** button on the Pico board.
3. While holding the button, plug the USB cable back in.
4. Release the button. Your computer should now show a new drive named **RPI-RP2**.

---

## Lesson 1 – Car Move & Turn

### 🚦 The Basics: How to Read the Code

The "Magic Command":

```cpp
Motor_M_Move(LeftFront, LeftRear, RightFront, RightRear);
```

- **Positive numbers (1 to 100):** Wheel goes forward.
- **Negative numbers (-1 to -100):** Wheel goes backward.
- **`delay(1000)`:** Wait for 1000 milliseconds (which is 1 second).

### 🏆 Challenge 1: The Speed Racer

**The Goal:** Make the car go twice as fast when moving forward, and super slow when moving backward.

**How-to:** Look at the first `Motor_M_Move` line. Change the 50s to a higher number (up to 100).

> **Tip:** Ask them: "If 100 is the fastest, what number would make the car crawl like a turtle?"

### 🕒 Challenge 2: The Long Haul

**The Goal:** Make the car drive forward for 3 seconds instead of just 1.

**How-to:** Find the `delay(1000);` line right after the forward command. Change the number inside the parentheses.

> **Tip:** Remind them that the computer counts in milliseconds. 1000 = 1 second, so 3000 = 3 seconds.

### 🔄 Challenge 3: The "Dizzy" Spinner

**The Goal:** Make the car spin in a circle in one spot (a "Zero Turn").

**How-to:** To spin, one side of the car needs to go forward while the other side goes backward.

The change:

```cpp
Motor_M_Move(50, 50, -50, -50);
```

> **Tip:** If the car just turns slightly, increase the delay so it keeps spinning longer!

---

## Lesson 2 – Matrix Display

Those two squares on the front of your car are like the robot's "eyes," and you get to tell them exactly what to do. Right now, your robot is programmed to blink, but you can change its personality just by tweaking the code.

Here are 5 missions for you to try!

### 🚀 Mission 1: The Fast-Blinker

The robot is currently blinking slowly. Let's make it look like it just drank a giant soda!

Find this line: `showArray(500);` (it's at the bottom in the `void loop`).

**The Goal:** Change 500 to 100.

**The Secret:** That number is "milliseconds." 1000 is one second. By making the number smaller, you're telling the robot to move faster!

### 😉 Mission 2: The Pirate Wink

Can you make the robot wink one eye while keeping the other one wide open?

Look at: `y_array` (that's the right eye).

**The Goal:** Copy the first line of numbers and paste them over the second line so they are exactly the same.

**The Secret:** If the right eye stays the same but the left eye (`x_array`) keeps changing, it will look like a wink!

### 🌙 Mission 3: Night Mode

The robot's eyes might be really bright. Let's dim them down so it can "see" in the dark without hurting its eyes.

Find this: `matrix.init(0x71);` (inside `void setup`).

**The Goal:** Right under that line, add this: `matrix.setBrightness(1);`

**The Secret:** You can use any number from 0 (off) to 15 (super bright). Try 15 to see the difference!

### 😠 Mission 4: Angry Robot

Let's make the eyes look like flat, grumpy lines.

Look at: `x_array` and `y_array`.

**The Goal:** Change one of the rows of numbers to this:

```cpp
0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00
```

**The Secret:** `0x00` means "lights off" and `0xFF` means "all lights on in this row." You just drew a thick line in the middle!

### 🎨 Mission 5: The Master Artist (Custom Face)

This is the hardest mission! Can you make the eyes look like diamonds? 💎

**The Goal:** Use these "secret codes" for your array:

```cpp
0x08, 0x1C, 0x3E, 0x7F, 0x7F, 0x3E, 0x1C, 0x08
```

**The Secret:** Each of those codes represents one row of the 8x8 grid. If you put those in, the robot will have diamond-shaped eyes.

### 💡 Top Secret Coding Tips

- **The "Comma" Rule:** Make sure every number has a comma after it, except for the very last one in the row. If you forget a comma, the robot gets a "headache" and won't run!
- **The "Zapped" Rule:** If you make a mistake and the code won't upload, don't worry! Just hit the "Undo" button (**Ctrl+Z**) to go back to when it worked.
- **The "Drawing" Trick:** If you want to draw your own face, ask a grown-up to find an "8x8 LED Matrix Tool" online. You click the dots, and it gives you the `0x` codes for your robot!

---

## Lesson 3 – Ultrasonic Ranging

### 🛰️ The "Head" and the "Ears"

The robot is using two main parts for this:

- **The Servo:** This is a little motor that acts like a neck. It can turn to specific angles (like 30°, 90°, or 150°).
- **The Ultrasonic Sensor:** These are the two round things on the front that look like eyes, but they are actually ears! They send out a "ping" of sound and listen for it to bounce back to see how far away an object is.

### 📝 What the Code is Doing

When you run this program, here is the robot's "to-do" list:

**1. The Startup (`setup`)**

- It wakes up the Serial Port so it can talk to your computer screen.
- It gets the sensor and the "neck" (servo) ready.
- It looks straight ahead by setting the angle to 90°.

**2. The Scouting Loop (`loop`)**

The robot will do these four steps over and over again forever:

- **Look Left:** It turns its head to 150° and "pings" to see how far away things are on the left.
- **Look Center:** It turns back to 90° and checks the distance straight ahead.
- **Look Right:** It turns its head to 30° and checks the distance on the right.
- **Look Center again:** It turns back to 90° to finish the scan.

### 🚀 Mission 1: The Fast Scanner 🏃‍♂️

The robot is currently scanning quite slowly. Let's see if you can make it "watch" the room like a fast-moving security camera!

**The Goal:** Make the robot look left, center, and right much faster.

**The How-to:** Find all the `delay(500);` lines. Change that 500 to 200.

**The Secret:** Lowering the delay makes the robot wait less time between movements. If you go too low (like 50), the robot's neck might get a bit shaky!

### 🚀 Mission 2: The Owl Turn 🦉

Owls can turn their heads really far around. Your robot is currently only looking a little bit to the left and right.

**The Goal:** Make the robot look as far left and as far right as it possibly can.

**The How-to:** Change the angle 150 to 180 (maximum left) and change the angle 30 to 0 (maximum right).

**The Secret:** Servos usually move from 0 to 180. 90 is always the middle!

### 🚀 Mission 3: The Wall Alarm 🚨

Let's use the Serial Monitor on your computer to play a game of "Don't Touch!"

**The Goal:** Use your hand to make the robot "yell" at you through the computer screen when you get too close.

**The How-to:** Open your Serial Monitor. Look at the distances. Try to find a way to make the robot say "TOO CLOSE!" when the distance is less than 10.

**The Hint:** This is a bit of a trick! You'll need to use an `if` statement. If that's too hard yet, just try to move your hand until the Serial Monitor shows exactly 5. It's harder than it looks!

> **Tip 1 — The "If" Secret:** To make the robot think, we use an `if` statement. It looks like this:
>
> ```cpp
> if (Get_Sonar() < 10) {
>   Serial.print("TOO CLOSE!");
> }
> ```
>
> This tells the robot: "If the distance is less than 10, shout at me!"
>
> **Tip 2 — Where to Put It:** Place your `if` code right after the `Get_Sonar` line. That way, the robot checks the distance and immediately decides if it needs to yell.
>
> **Tip 3 — The Ghost Numbers:** Sometimes the sensor sees "0" or a huge number for a split second if the sound wave bounces weirdly. Don't worry if the alarm goes off once by mistake — that's just "sensor noise!"

### 🚀 Mission 4: The "Yes" Man ✅

Can you make your robot shake its head "yes" instead of "no"?

**The Goal:** Change the code so the robot looks up and down instead of left and right.

**The How-to:** Actually... check your robot! Is the sensor mounted to look side-to-side or up-and-down?

**The Secret:** If your servo is mounted sideways, you can't make it look "up." To finish this mission, you have to re-write the code so it only uses two angles: 70 and 110, switching back and forth fast!

> **Tip 1 — The Small Shake:** If your robot's head moves side-to-side, a "nod" is just a very small, fast movement. Instead of going from 30 to 150, try just going from 80 to 100. It will look like the robot is shivering or saying a quick "yes!"
>
> **Tip 2 — Speed is Key:** To make it look like a real nod, you need to change the delay. Try `delay(100);`. If the delay is too long, it looks like a slow turn. If it's short, it looks like a quick gesture!
>
> **Tip 3 — Check the Neck:** Before you hit upload, move the robot's head gently with your hand. Does it move easily? If it feels stuck, don't force it — just make sure your code stays within the numbers that feel "smooth."

### 🚀 Mission 5: The Guard Dog 🐕

A guard dog stays still until it sees something move.

**The Goal:** Make the robot stay at 90 degrees (looking straight) for a long time, then do one quick "scan" (left and right) and go back to waiting.

**The How-to:** At the very end of your `void loop()`, add a giant delay: `delay(5000);`.

**The Secret:** 5000 is 5 seconds. Your robot will look around, then "stare" straight ahead for 5 seconds before checking its surroundings again.

> **Tip 1 — The "Loop" Order:** Remember, the robot reads the `void loop` like a book — from top to bottom. If you put the `delay(5000)` at the very bottom, it will finish its whole scan before it "takes a nap" for 5 seconds.
>
> **Tip 2 — The Statue Move:** If you want it to look even more like a guard dog, make sure your very last command before the long delay is `Servo_1_Angle(90);`. This ensures it's looking straight ahead while it waits.
>
> **Tip 3 — Change the Nap Time:** If 5 seconds feels too long, you can change the "nap" whenever you want.
> - 2000 = 2 second nap.
> - 10000 = 10 second nap!

### 💡 Engineering Pro-Tips

- **The "90" Rule:** Whenever you get confused about where the robot is looking, set the angle to 90. That is the "Home" position.
- **Watch the Wires:** When you make the robot turn to 0 or 180, make sure the wires don't get tangled or pulled too tight.
- **The Serial Monitor:** Remember to click the little Magnifying Glass or Serial Monitor icon in your coding program, otherwise, you won't be able to see the distance numbers!

---

## Lesson 4 – IR Follow

Think of this code as the "Brain" of your robot. It follows a simple loop: **Look, Decide, Act.**

Here is the secret behind how it thinks:

**1. The "Eyes" (Reading the Sensors)**

The robot has three sensors on the bottom. In the code, `Track_Read()` tells the robot to look at the ground. It stores what it sees as a "Sensor Value."

- 0 means it sees the white floor.
- 1 means it sees the black line.

Because there are three sensors, they combine into a 3-bit "secret code" (like 010). The computer turns that code into a single number (0 through 7) so it can make a quick decision.

**2. The "Decision" (The Switch Case)**

The `switch` section is like a giant list of rules. The robot asks: "Which number did my eyes just see?"

- If it's **2** (010): The line is right in the middle! Rule: Drive straight.
- If it's **4 or 6** (100 or 110): The line is sliding to the left! Rule: Turn left to get back on it.
- If it's **1 or 3** (001 or 011): The line is sliding to the right! Rule: Turn right.
- If it's **7** (111): It sees a big black wall or finish line! Rule: Stop moving.

**3. The "Legs" (The Motors)**

Once the brain decides what to do, it uses `Motor_M_Move` to tell the wheels how to spin.

- Positive numbers make the wheels go forward.
- Negative numbers make them go backward.
- To turn left, the robot actually makes the left wheels go backward and the right wheels go forward — just like how you spin a 360 on a skateboard or a swivel chair!

**4. The "Face" (The Emotions)**

While the legs are moving, the code uses commands like `showArrow` or `eyesBlink`. This doesn't help the car drive, but it tells you what the car is thinking! If it sees the line and shows an arrow, you know the "Brain" is working correctly.

**Basically:** The robot spends its whole life asking "Where is the black line?" and "How fast should I spin my wheels to stay on it?" millions of times every second!

### Challenge 1: The "Top Speed" Test

Make the robot go around the track as fast as possible.

### Challenge 2: Reverse

Make the robot reverse backwards in a straight line whenever it loses the line.

### Challenge 3: The "Backwards"

Make the robot go around the track backwards.

---

## Lesson 5 – Obstacle Avoidance (Bat Sonar!)

Think of this code as your robot's "Spidey-Sense." Instead of looking at the floor, it is now looking straight ahead to make sure it doesn't crash into a wall, a chair, or the family dog! It follows a new loop: **Shout, Listen, React.**

Here is the secret behind how it avoids crashing:

**1. The "Bat Ears" (The Ultrasonic Sensor)**

The robot has a piece on the front that looks like two big eyes, but they are actually a tiny speaker and a microphone! In the code, a command like `readUltrasonic()` tells the robot to do exactly what bats and submarines do:

- **The Shout:** It sends out a super-high invisible squeak (too high for our human ears to hear).
- **The Listen:** It waits to hear the echo bounce back off a wall.

**2. The "Math Brain" (Calculating Distance)**

The computer uses a built-in stopwatch to count how fast the echo comes back.

- A short time means the wall is right in front of it!
- A long time means the path ahead is totally clear.

The code does a quick math trick to turn that time into a simple number you can read, like "15 centimeters."

**3. The "Decision" (The If/Else Block)**

Instead of a giant switch list, this code usually uses a simple If/Else rule to keep the robot safe. The robot constantly asks: "Am I about to bump my nose?"

- If the distance is less than 15 cm: **Danger!** Rule: Hit the brakes immediately.
- Else (if it's greater than 15 cm): **The coast is clear!** Rule: Keep driving forward.

**4. The "Escape Plan" (The Motors)**

When the robot gets too close to a wall, stopping isn't enough. It has to escape! It uses the `Motor_M_Move` block to perform a special combo:

- First, both wheels get negative numbers to back away from the wall.
- Next, one wheel goes forward and the other goes backward to spin the robot around.
- Now, it faces a brand new direction and can go back to exploring!

**Basically:** The robot spends its whole life shouting into the room and listening for echoes so it never stubs its robotic toe!

---

## Lesson 6 – Light Tracking

### Level 1: The Speed Hacker (Warm-up)

Right now, the robot's "brain" is reading the light sensors and printing the numbers to the screen twice every second.

**The Mission:** Change the code so the robot reads the light super fast — ten times a second! Then, change it so it only reads the light once every two seconds.

**What this teaches:** Understanding the `delay()` function and how milliseconds work (1000 ms = 1 second).

**The Solution:** Find `delay(500);` and change it to `delay(100);` (for fast) and `delay(2000);` (for slow).

### Level 2: The Light Detective (Logic & If/Else)

Instead of just spitting out raw numbers, let's make the robot actually understand the numbers.

**The Mission:** Add an if/else statement inside the `loop()`. If the left sensor's number is bigger than the right sensor's number, tell the computer to print "The light is on the Left!" Otherwise, print "The light is on the Right!"

**What this teaches:** Comparing two variables using the greater-than `>` operator.

**The Hint:** He will need to add something like this before the delay:

```cpp
if (getLeftPhotosensitiveADCValue > getRightPhotosensitiveADCValue) {
  Serial.println("The light is on the Left!");
} else {
  Serial.println("The light is on the Right!");
}
```

### Level 3: The Dark Room Alarm (Variables & Math)

Let's see if he can create a brand new variable and use it to trigger an alarm.

**The Mission:** Create a new `int` variable inside the loop called `totalLight`. Make it equal to the left sensor plus the right sensor. If `totalLight` drops below 100, print "WARNING: IT IS TOO DARK!"

**What this teaches:** Variable declaration inside a scope, basic arithmetic (`+`), and threshold logic.

**Good to know:** On this sensor, LOWER numbers mean DARKER and HIGHER numbers mean BRIGHTER — the opposite of what you might expect! Roughly: under 10 = pitch black, under 50 = very dark, around 250 = bright, and 1000+ = super bright (like a torch shining right on it).

**The Hint:**

```cpp
int totalLight = getLeftPhotosensitiveADCValue + getRightPhotosensitiveADCValue;

if (totalLight < 100) {
  Serial.println("WARNING: IT IS TOO DARK!");
}
```
