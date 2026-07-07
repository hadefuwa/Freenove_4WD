// ---------------------------------------------------------------------
// WHAT IS THIS FILE?
// ---------------------------------------------------------------------
// This is NOT code that runs on the robot car! It's a separate helper
// program written in "Processing" (a language similar to Java) that runs
// on a computer. It shows two clickable 8x8 grids on screen so you can
// draw a face with your mouse, then press "GETCODE" to print out the
// matching 0x.. hex numbers - the exact same kind of numbers used in
// x_array/y_array back in 01.5_Matrix.ino! It's basically a picture
// editor that speaks the LED matrix's "hex language" for you, so you
// don't have to work out the binary by hand.
//
// It uses the "ControlP5" library, which provides ready-made buttons,
// sliders and clickable grids (here called "Matrix" objects) for
// building simple on-screen tools.
// ---------------------------------------------------------------------

import controlP5.*; //<>//
ControlP5 cp5;
int nx =16;
int ny = 8;
int []cn=new int[20];

// ax/ay: store the (x, y) grid positions of every LED the user has
// turned on, across every saved frame. They're sized huge (2600*2) just
// so there's always plenty of room, like an oversized notebook.
int []ax=new int[2600*2];
int []ay=new int[2600*2];

Matrix matrix1;
Matrix matrix2;
Matrix matrix3;
Button B1, B2, B3;
int mn;
int timect=0;
int te=20;
int tn=0;
boolean state=false;
int barn;
int matrixplay=0;
int speed;
int moven=0;

// setup(): runs once when the Processing program starts. It builds the
// whole on-screen tool - the two clickable LED grids, the speed slider,
// and all the buttons (PREVIOUS, CLEAR, ON_ALL, BlinkShow, GETCODE...).
void setup() {
  size(500, 460);
  mn=nx*ny;
  cp5 = new ControlP5(this);
  cp5.addButtonBar("bar")
    .setPosition(50, 40)
    .setSize(400, 20)
    .addItems(split("a b c d e f g h i j k l m n o p q r s t", " "))
    ;
  matrix1=cp5.addMatrix("myMatrix1")
    .setPosition(50, 100)
    .setSize(15*nx, 15*ny)
    .setGrid(nx, ny)
    .setGap(1, 1)
    .setInterval(0)
    .setMode(ControlP5.MULTIPLES)
    .setColorBackground(color(120))
    .setBackground(color(40))
    ;
  matrix1.stop();

  matrix2=cp5.addMatrix("myMatrix2")
    .setPosition(50, 150+15*ny)
    .setSize(15*nx, 15*ny)
    .setGrid(nx, ny)
    .setGap(1, 1)
    .setInterval(0)
    .setMode(ControlP5.MULTIPLES)
    .setColorBackground(color(120))
    .setBackground(color(40))
    ;
  matrix2.stop();

  //cp5.addColorWheel("colorplate", 550, 50, 200 ).setRGB(color(128, 0, 255))
  //   .setColorBackground(color(40));

  cp5.addSlider("SPEED")
    .setPosition(350, 350)
    .setRange(0, 60)
    .setSize(100, 20)
    .setValue(20)
    ;
  cp5.getController("SPEED").getCaptionLabel().align(ControlP5.CENTER, ControlP5.BOTTOM_OUTSIDE).setPaddingX(0);

  // use setMode to change the cell-activation which by 
  // default is ControlP5.SINGLE_ROW, 1 active cell per row, 
  // but can be changed to ControlP5.SINGLE_COLUMN or ControlP5.MULTIPLES

  int bt=25;
  int bpx=350;
  B1=cp5.addButton("PREVIOUS")
    .setValue(0)
    .setPosition(350, 90)
    .setSize(100, bt)
    ;
  cp5.addButton("CLEAR")
    .setValue(0)
    .setPosition(350, 130)
    .setSize(100, bt)
    ;
  B2=cp5.addButton("ON_ALL")
    .setValue(100)
    .setPosition(bpx, 170)
    .setSize(100, bt)
    ;
  B3=cp5.addButton("CLEAR_ALL")
    .setPosition(bpx, 210)
    .setSize(100, bt)
    .setValue(0)
    ;
  cp5.addButton("BlinkShow")
    .setPosition(bpx, 280)
    .setSize(100, bt)
    .setValue(0)
    ;
  cp5.addButton("MoveShow")
    .setPosition(bpx, 310)
    .setSize(100, bt)
    .setValue(0)
    ;
  cp5.addButton("GETCODE")
    .setPosition(bpx, 400)
    .setSize(100, bt)
    .setValue(0)
    ;
  noStroke();
  smooth();
}

// draw(): in Processing, this runs automatically many times per second
// (like loop() in Arduino). It repaints the background, records whatever
// the user has drawn on matrix1, and - if a preview animation is
// running - shows the blink or move animation on matrix2.
void draw() {
  background(30);
  getmatrixdata();
  if (matrixplay==1) {
    showmatrixdata1();
  }
  if (matrixplay==2) {
    showmatrixdata2();
  }
  if (timect>=20*te) {
    timect=0;
  }
  timect++;
}

// showmatrixdata1(): plays back the saved frames on matrix2 one at a
// time (like flipping through a picture book), pausing "speed" ticks
// between each frame - this previews the simple blink animation.
void showmatrixdata1() {
  te=speed;
  if (timect==(tn+1)*te) {
    matrix2.clear();
    for (int i=mn*tn; i<cn[tn]+mn*tn; i++) {
      matrix2.set(ax[i], ay[i], true);
    }
    tn++;
    if (cn[tn]==0||tn>19) {
      timect=0;
      tn=0;
    }
  }
}

// showmatrixdata2(): previews a "sliding" animation instead of a plain
// blink - it gradually shifts one frame's dots sideways (using "moven")
// while the next frame's dots slide in from the other side, so the
// picture appears to scroll across the matrix.
void showmatrixdata2() {
  if (timect==speed) {
    matrix2.clear();

    for (int i=mn*tn; i<cn[tn]+mn*tn; i++) {
      if (ax[i]-moven>=0) {
        matrix2.set(ax[i]-moven, ay[i], true);
      }
    }
    for (int i=mn*(tn+1); i<cn[tn+1]+mn*(tn+1); i++) {
      if ((ax[i]+nx-moven>=0)&&(ax[i]+nx-moven<nx)) {
        matrix2.set(ax[i]+nx-moven, ay[i], true);
      }
    }

    moven++;
    if (moven==nx) {
      moven=0;
      tn++;
    }
    if (cn[tn]==0||tn>19) {
      tn=0;
    }
    timect=0;
  }
}

// getmatrixdata(): checks every cell of the on-screen drawing grid
// (matrix1) and remembers the (x, y) position of each LED the user has
// clicked on, into the ax/ay lists, for whichever frame slot "barn" is.
void getmatrixdata() {
  cn[barn]=0;
  for (int x=0; x<nx; x++) {
    for (int y=0; y<ny; y++) {
      state= matrix1.get(x, y);
      if ( state==true) {
        ax[cn[barn]+barn*mn]=x;
        ay[cn[barn]+barn*mn]=y;
        cn[barn]++;
      }
    }
  }
}

// bar(n): called when the user clicks a letter on the button-bar at the
// top (frame slots a, b, c...). Switches the drawing grid to show
// whichever saved frame number "n" corresponds to.
// Parameter: n - the frame slot index to switch to and display.
void bar(int n) {
  barn=n;
  matrix1.clear();
  for (int i=mn*barn; i<cn[n]+mn*barn; i++) {
    matrix1.set(ax[i], ay[i], true);
  }
}

// PREVIOUS(): button handler for the "PREVIOUS" button. Clears the
// drawing grid and redraws whatever was saved in the frame just before
// the current one, so you can flip backwards through your frames.
void PREVIOUS() {
  matrix1.clear();
  if (barn>0) {
    for (int i=mn*(barn-1); i<cn[barn-1]+mn*(barn-1); i++) {
      matrix1.set(ax[i], ay[i], true);
    }
  }
}
// ON_ALL(): button handler for "ON_ALL". Turns every single LED on the
// drawing grid on, useful as a quick starting point or test pattern.
void ON_ALL() {
  for (int i=0; i<nx; i++) {
    for (int j=0; j<ny; j++) {
      matrix1.set(i, j, true);
    }
  }
}

// BlinkShow(): button handler that toggles the simple blink preview
// on/off (calls showmatrixdata1() from within draw() while it's active).
void BlinkShow() {
  timect=0;
  if (matrixplay==1) {
    matrixplay=0;
  } else {
    matrixplay=1;
  }
}

// MoveShow(): button handler that toggles the sliding/scrolling preview
// on/off (calls showmatrixdata2() from within draw() while it's active).
void MoveShow() {
  timect=0;
  if (matrixplay==2) {
    matrixplay=0;
  } else {
    matrixplay=2;
  }
}

// GETCODE(): the important one! Converts every drawn frame into the
// same hex byte format used by x_array/y_array in 01.5_Matrix.ino.
// For each row, it adds up powers of 2 (1,2,4,8,16,32,64,128) for every
// lit LED in that row - exactly how binary bits combine into one byte -
// then prints that row's total as a "0x.." hex number. Copy/paste the
// printed numbers straight into the .ino file's arrays to use your
// drawing on the real robot.
void GETCODE() {
  println("------------------------------get HEX------------------------------------------------------------------");
  int cnn=0;
  for (int i=0; i<20; i++) {
    if (cn[i]==0) {
      break;
    }
    cnn++;
  }
  String hex;
  int bn=0;
  for (int k=0; k<cnn; k++ ) {
    for (int j=0; j<ny; j++) {
      for (int i= mn*k; i<cn[k]+mn*k; i++) {
        if (ay[i]==j ) {
          bn=bn+(int)pow(2, (7-ax[i]));      
        }
      }
      hex=Integer.toHexString((int)bn).toUpperCase();
      if (bn<10) {
        print("0x0"+hex+",");
      } else {      
        print("0x"+hex+",");
      }
      bn=0;
    }

  }
      println("---------x");
  for (int k=0; k<cnn; k++ ) {
    for (int j=0; j<ny; j++) {
      for (int i= mn*k; i<cn[k]+mn*k; i++) {
        if (ay[i]==j &&ax[i]>7) {
          bn=bn+(int)pow(2, (15-ax[i]));      
        }
      }
      hex=Integer.toHexString((int)bn).toUpperCase();
      if (bn<10) {
        print("0x0"+hex+",");
      } else {      
        print("0x"+hex+",");
      }
      bn=0;
    }
   
  } 
   println("---------y");
}


// CLEAR(): button handler that switches off every LED on the current
// drawing frame only (other saved frames are untouched).
void CLEAR() {
  matrix1.clear();
}

// CLEAR_ALL(): button handler that wipes the current drawing frame AND
// forgets every saved frame's data, resetting the whole tool.
void CLEAR_ALL() {
  matrix1.clear();
  for (int i=0; i<20; i++) {
    cn[i]=0;
  }
}

// SPEED(n): called automatically whenever the speed slider is moved.
// Stores the new animation speed and resets the animation timer.
// Parameter: n - the new speed value read from the slider.
void SPEED(int n) {
  speed=n;
  timect=0;
}
