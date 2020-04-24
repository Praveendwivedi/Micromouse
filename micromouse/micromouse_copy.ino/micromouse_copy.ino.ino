

//move_next_cell
//u_turn

#define max_wt 256
#define wt_start 256
#define max_dis 20
#define encl 2
#define encr 3
#define mr1 4
#define mr2 5
#define ml1 6
#define ml2 7
#define enr 10
#define enl 11
#define wall_limit 10
#include <NewPing.h>
#include<EEPROM.h>
/*
 * =========================================================
 *    praven
 *    =====================================================
 */
int wall_threshold = 70 ;
int front_threshold = 60 ;
//float P = 2.353 ;
//float D = 0.11167 ;
//float I = 0.4467 ;
float P = 0.25 ;
float D = 0.05 ;
float I = 0.1 ;
float oldErrorP ;
float totalError ;
int offset = 2 ;
int BaseSpeed = 80 ;
int BaseSpeedr = 100 ;
int BaseSpeedl = 100 ;
int lms ;
int rms ;
boolean frontwall ;
boolean leftwall ;
boolean rightwall ;
boolean first_turn ;
boolean rightWallFollow ;
boolean leftWallFollow ;
int rightMiddleValue=38;
int leftMiddleValue=38;
float errorP,errorI,errorD,errorE;


 
NewPing sonar_l(A0,A1,max_dis);
NewPing sonar_m(A2,A3,100);
NewPing sonar_r(A5,A4,max_dis);



int dis_left;
int dis_middle;
int dis_right;

float leftrot=0,rightrot=0;
void leftpulse()
{
  leftrot+=0.05;
}
void rightpulse()
{
 rightrot+=0.05;
}


// first maze_wall then maze_waeghts value stored in eeprom
int stack[256][2]; //maze_wall = 1xyzwv ||x = north|y = west|z = south|w = East|v = visited

int  bin[5];




struct wall_information
{
  bool front,left,right;
}cwall;






void dec_to_bin(int dec)
{
  /* N = 4
   * W = 3
   * S = 2
   * E = 1
   * V = 0
   */
  int i;
  for(i=0;i<5;i++)
  {
    bin[i] = dec%2;
    dec = dec/2;
  }
}



void maze_weight(char state[],int x,int y,int value)
{
  if (state=="assign")
    EEPROM.update(wt_start+x*16+y,value);
  else if(state=="add")
  {
    int a = EEPROM.read(wt_start+x*16+y);
    EEPROM.update(wt_start+x*16+y,value+a);
  }
  
}
int maze_weight_get(int x,int y)
{
  int a = EEPROM.read(wt_start+x*16+y);
  return a;
}
void maze_wall(char state[],int x,int y,int value)
{
  if(state=="assign")
    EEPROM.update(x*16+y,value);
  else if(state=="add")
  {
    int a = EEPROM.read(x*16+y);
    EEPROM.update(x*16+y,value+a);
  }
}
int maze_wall_get(int x,int y)
{
  int a = EEPROM.read(x*16+y);
  return a;
}
void read_walls()                                                         // reading values from sensors and assigning it to cwall
{ 
  cwall.front = 0;
  cwall.right = 0;
  cwall.left = 0;
  if (dis_left<wall_limit)
      cwall.left = 1;
    
  if (dis_middle<wall_limit)
      cwall.front = 1;
    
  if (dis_right<wall_limit)
      cwall.right = 1;
    
}



void add(int x , int y)                                                               //adding given value's coordinates to stack   
{
  int i = 0;
  if(!(   (x == 7 && (y == 7 || y == 8))  || (x == 8 && (y == 7 || y== 8))     ))
  { 
    for(i = 0;i<256;i++)
    {
      if(stack[i][0] == 16)
      {
        stack[i][0] = x;
        stack[i][1] = y;
        break;

      }
    }
  }
}

void remove(int x, int y)                                      //removing checked value from stack
{
  int i;
  for(i = 0;i<256;i++)
    {
      if(stack[i][0] == x)
      {
        if(stack[i][1] == y)
        {
          stack[i][0] = 16;
          stack[i][1] = 16;
          break;

        }  
      }
   }
}


int check()                                     //for checking whether stack is empty or nor and if not then giving the location of stored value.
{
  int i;
  for(i = 0;i<256;i++)
  {
    if(stack[i][0] != 16)
      return (i+1);

  }
  if(i == 256)
    return 0;
}


struct bot
{
  int x,y;
  char cur_direction;
  char prev_direction;
};
struct bot micromouse;




void setup()
{
  Serial.begin(9600);
  pinMode(encl,INPUT);
  pinMode(encr,INPUT);
  attachInterrupt(digitalPinToInterrupt(encl), leftpulse,RISING);
  attachInterrupt(digitalPinToInterrupt(encr), rightpulse, RISING);
  pinMode(ml1,OUTPUT);
  pinMode(ml2,OUTPUT);
  pinMode(mr2,OUTPUT);
  pinMode (mr1,OUTPUT);
  pinMode(enl,OUTPUT);
  pinMode(enr,OUTPUT);
//  for(int i=0;i<16;i++)
// {
//  for(int j=0;j<16;j++)
//  {
//    maze_wall("assign",i,j,32);
//    
//  }
//
// }
 flood_fill('g');
 for(int i=0;i<256;i++)
 {
    stack[i][0]=16;
    stack[i][1]=16;
 }
                                                       //wall assign
 
 for(int i=0;i<16;i++)
 {
    maze_wall("add",0,i,8);
    maze_wall("add",15,i,2);
    maze_wall("add",i,0,4);
    maze_wall("add",i,15,16);
 }
 
                                                       //weight assign
 int a=14;
 for(int i=0;i<16;i++)       // assigning the initial weights of the cubes...
 {
   for(int j=0;j<16;j++)
   {
     maze_weight("assign",i,j,a);
     if(j<7)
       a-=1;
     else if(j>7 && j<15)
       a+=1;
   if(i<7)
     a-=1;
   else if(i>7)
     a+=1;
   }
 }
 maze_wall("assign",0,0,31);      //blocking the start cube.
 maze_weight("assign",0,0,100);            //assignings weight to start cube.
 delay(2000);
}
void loop()
{
  
  //algo
  int i,j,b=0;  //  a for  predefining weight of cubes    i for x(horizonntal)    j for y(vertical)    cwt for current weight of pressent cube
  micromouse.x = 0;             //dikkat aa rhi thi is liye assign kiya hai
  micromouse.y = 1;
  micromouse.cur_direction = 'N';    //pehleto north me hi jaaunga na.
  micromouse.prev_direction = 'N';
  while (!((micromouse.x == 7 && micromouse.y == 7) or 
        (micromouse.x == 7 && micromouse.y == 8) or 
        (micromouse.x == 8 && micromouse.y == 7) or 
        (micromouse.x == 8 && micromouse.y == 8) )) //if reached destination then terminate the loop
        {
          solve('I');
          movement();
          micromouse.prev_direction = micromouse.cur_direction;
        }
  flood_fill('r');
  while(!(micromouse.x == 0 && micromouse.y == 0))
  {
    solve('F');
    movement();
    micromouse.prev_direction = micromouse.cur_direction;
  }
}


/*==============================================================================
 *                                   ALGO
 *===============================================================================                                   
 */
void flood_fill(char state)
{
  int x1 = micromouse.x;
  int y1 = micromouse.y;
  int inf =255;

  int weight = 0;       
  int count1=1;
  int count2=0;
  int pos = 0;
  int next_free=1;
  for(int i =0;i<16;i++)
  {
    for(int j=0;j<16;j++)
    {
        if(maze_wall_get(i,j)%2 !=0 )
            maze_wall("add",i,j,-1);
        maze_weight("assign",i,j,inf);
    }
  }
            
            
    if(state=='r')
    {    
      stack[0][0]=x1;
      stack[0][1]=y1;
      maze_wall("add",0,0,1);
    
    }
    else if(state=='g')
    {
      stack[0][0]=7;
      stack[0][1]=7;
     
      stack[0][0]=7;
      stack[0][1]=8;
      
      stack[0][0]=8;
      stack[0][1]=7;
      
      stack[0][0]=8;
      stack[0][1]=8;

      count1=4;
      next_free=4;
      maze_wall("add",7,7,1);
      maze_wall("add",7,8,1);
      maze_wall("add",8,7,1);
      maze_wall("add",8,8,1);
    }
    

    int x;
    int y;
    
    while (check())
    {
        x= stack[pos][0];
        y= stack[pos][1];
        maze_weight("assign",x,y,weight);
        dec_to_bin(maze_wall_get(x,y));     //do bit demasking and assign binary number to n
  
         int a=0;
         int b=1;
         
         for(int i = 4; i>0; i--)                 //finding minimum values among the neighbours
         {
           if(bin[i] == 0 && maze_wall_get(x+a,y+b)%2 ==0 )
           {
             
               stack[next_free][0]=x+a;
               stack[next_free][1]=y+b;
               maze_wall("add",x+a,y+b,1);
               count2++;
               next_free++;
               
           }
           a=a+b;
           b=a-b;
           a=a-b;
           if(i != 3)
           {
             a = -a;
             b = -b;
           }
         }
        count1 -= 1;
        if (count1==0)
        {
            count1=count2;
            count2=0;
            weight+=1;
        }
        pos++;
    }
  
    for(int i =0;i<16;i++)
    {
      for(int j=0;j<16;j++)
      {
          if(maze_wall_get(i,j)%2 !=0 )
              maze_wall("add",i,j,-1);
      }
    }
    
}


void solve(char state)
{
  int cwt = maze_weight_get(micromouse.x,micromouse.y);   //assigning weight to current state
  read_walls();                                        // here i am assuming that we are assigning current values to cwall in read_walls function
  /* N = 4
   * W = 3
   * S = 2
   * E = 1
   * V = 0
   */
   int minimum = 0,i,x,y;

//=================================================================================
//                                   NORTH
//===============================================================================

   
  if( micromouse.cur_direction =='N')                //checking for north direction
  {
     if( (cwall.front) and (micromouse.y < 15))
     {
       dec_to_bin(maze_wall_get(micromouse.x,micromouse.y+1));     //+1 for north neighbour
  
       if(bin[2] == 0)//south
         maze_wall("add",micromouse.x,(micromouse.y)+1,4);
     }
     if( (cwall.right) and (micromouse.x <15))
     {
       dec_to_bin(maze_wall_get(micromouse.x+1,micromouse.y));    //+1 for east neighbour
  
       if(bin[3] == 0)//west
         maze_wall("add",(micromouse.x)+1,micromouse.y,8);
     }
     if( (cwall.left) and (micromouse.x > 0))
     {
       dec_to_bin(maze_wall_get((micromouse.x)-1,micromouse.y));    //-1 for west neighbour
  
       if(bin[1] == 0)//east of neighbour cell
         maze_wall("add",(micromouse.x)-1,micromouse.y,2);
     } 
     maze_wall("assign",micromouse.x,micromouse.y,1 + 16*(cwall.front) + 2*(cwall.right) + 8*(cwall.left));          //bit masking detected walls for current cube
  
     
     if(     (!(cwall.front))   and   (maze_weight_get(micromouse.x,micromouse.y+1)<cwt)  )                      //preparing values for next cube
       micromouse.y+=1;
     else if(    (!(cwall.right))   and   (maze_weight_get((micromouse.x)+1,micromouse.y)<cwt)  )
     {
         micromouse.x+=1;
         micromouse.cur_direction ='E';
     }
     else if(    (!(cwall.left))   and   (maze_weight_get((micromouse.x)-1,micromouse.y)<cwt)  )
     {
         micromouse.x-=1;
         micromouse.cur_direction ='W';
     }
 
     else
     {
       add(micromouse.x,micromouse.y);    //add coordinates of current cube to stack
       minimum = 256; 
  
       while(check())                       //check whether stack is empty or not
       {
         
         i = 0;
         i = check();                         //take first non zero value from stack
  
         if(i == 0);
         {
           break;                         //if there is nothing in stack, then do nothing and run while loop again
         }
         x = stack[i-1][0];                    //assign coordinates of first value from stack to x and y 
         y = stack[i-1][1];
         if(state=="I")
         {
           if((x==7 || x==8)&& (y==7||y==8))
           {
            remove(x,y);
            continue;
           }
         }
         dec_to_bin(maze_wall_get(x,y));     //do bit demasking and assign binary number to n
  
         int a=0;
         int b=1;
         int c=0;
         for(i = 4; i>0; i--)                 //finding minimum values among the neighbours
         {
           if(bin[i] == 0)
           {
             if(maze_weight_get(x+a,y+b)<minimum)
               minimum = maze_weight_get(x+a,y+b);
               c++;
           }
           a=a+b;
           b=a-b;
           a=a-b;
           if(i != 3)
           {
             a = -a;
             b = -b;
           }
         }
        if(c==0)
        {
          remove(x,y);
          continue;
        }
        if(minimum<max_wt)
        {
         if(maze_weight_get(x,y)-1 !=minimum)
         {
           maze_weight("assign",x,y,minimum+1);                // upgrade given cube's weight and ad all neighbours to stack
           if(x-1>0)
             add(x-1,y);
           if(x+1<16)
             add(x+1,y);
           if(y-1>0)
             add(x,y-1);
           if(y+1>0)
             add(x,y+1);
         }
        }
        remove(x,y);                                       //remove checked cube's coordinates from stack
      }
      int x1 = micromouse.x;
      int y1 = micromouse.y;
      if (!(cwall.front) and maze_weight_get(x1,y1+1)<maze_weight_get(x1,y1))
        micromouse.y++;
      else if(    (!(cwall.right))   and   (maze_weight_get((micromouse.x)+1,micromouse.y)<cwt)  )
      {
         micromouse.x+=1;
         micromouse.cur_direction ='E';
      }
      else if(    (!(cwall.left))   and   (maze_weight_get((micromouse.x)-1,micromouse.y)<cwt)  )
      {
         micromouse.x-=1;
         micromouse.cur_direction ='W';
      }
      else
      {
         micromouse.y-=1;
         micromouse.cur_direction ='S';
      }
    }    
  }


//=================================================================================
//                                   EAST
//===============================================================================



  
  else if( micromouse.cur_direction =='E')                //checking for east direction 
  {
    if( (cwall.left) and (micromouse.y < 15))
    {
      dec_to_bin(maze_wall_get(micromouse.x,(micromouse.y)+1));     //+1 for north neighbour
  
      if(bin[2] == 0)//south
         maze_wall("add",micromouse.x,(micromouse.y)+1,4);
    }
    if( (cwall.front) and (micromouse.x <15))
    {
      dec_to_bin(maze_wall_get(micromouse.x+1,micromouse.y));    //+1 for east neighbour
  
       if(bin[3] == 0)//west
         maze_wall("add",(micromouse.x)+1,micromouse.y,8);
    }
    if( (cwall.right) and (micromouse.y >0))
    {
      dec_to_bin(maze_wall_get(micromouse.x,micromouse.y-1));    //+1 for east neighbour
  
       if(bin[4] == 0)//west
         maze_wall("add",micromouse.x,micromouse.y-1,16);
    }
    
    maze_wall("assign",micromouse.x,micromouse.y,1 + 2*(cwall.front) + 4*(cwall.right) + 16*(cwall.left));          //bit masking detected walls for current cube
    
    if(     (!(cwall.front))   and   (maze_weight_get((micromouse.x)+1,micromouse.y)<cwt)  )
      micromouse.x+=1;
    else if(    (!(cwall.right))   and   (maze_weight_get(micromouse.x,micromouse.y-1)<cwt)  )
    {
        micromouse.y-=1;
        micromouse.cur_direction ='S';
    }
    else if(    (!(cwall.left))   and   (maze_weight_get(micromouse.x,micromouse.y+1)<cwt)  )
    {
        micromouse.y+=1;
        micromouse.cur_direction ='N';
    }
    
    else
     {
       add(micromouse.x,micromouse.y);    //add coordinates of current cube to stack
       minimum = 256; 
  
       while(check())                       //check whether stack is empty or not
       {
         
         i = 0;
         i = check();                         //take first non zero value from stack
  
         if(i == 0);
         {
           break;                         //if there is nothing in stack, then do nothing and run while loop again
         }
         x = stack[i-1][0];                    //assign coordinates of first value from stack to x and y 
         y = stack[i-1][1];
         if(state=="I")
         {
           if((x==7 || x==8)&& (y==7||y==8))
           {
            remove(x,y);
            continue;
           }
         }
         dec_to_bin(maze_wall_get(x,y));     //do bit demasking and assign binary number to n
  
         int a=0;
         int b=1;
         int c=0;
         for(i = 4; i>0; i--)                 //finding minimum values among the neighbours
         {
           if(bin[i] == 0)
           {
             if(maze_weight_get(x+a,y+b)<minimum)
               minimum = maze_weight_get(x+a,y+b);
               c++;//check if cell has all side walls 
           }
           a=a+b;
           b=a-b;
           a=a-b;
           if(i != 3)
           {
             a = -a;
             b = -b;
           }
         }
        if(c==0)
        {
          remove(x,y);
          continue;
        }
        if(minimum<max_wt)
        {
         if(maze_weight_get(x,y)-1 !=minimum)
         {
           maze_weight("assign",x,y,minimum+1);                // upgrade given cube's weight and ad all neighbours to stack
           if(x-1>0)
             add(x-1,y);
           if(x+1<16)
             add(x+1,y);
           if(y-1>0)
             add(x,y-1);
           if(y+1>0)
             add(x,y+1);
         }
        }
        remove(x,y);                                       //remove checked cube's coordinates from stack
      }
      
      int x1 = micromouse.x;
      int y1 = micromouse.y;
      
      if(     (!(cwall.front))   and   (maze_weight_get((micromouse.x)+1,micromouse.y)<cwt)  )
        micromouse.x+=1;
      else if(    (!(cwall.right))   and   (maze_weight_get(micromouse.x,micromouse.y-1)<cwt)  )
      {
          micromouse.y-=1;
          micromouse.cur_direction ='S';
      }
      else if(    (!(cwall.left))   and   (maze_weight_get(micromouse.x,micromouse.y+1)<cwt)  )
      {
          micromouse.y+=1;
          micromouse.cur_direction ='N';
      }
      else
      {
         micromouse.x-=1;
         micromouse.cur_direction ='W';
      }
      
    }
  }

//=================================================================================
//                                   WEST
//===============================================================================




  else if( micromouse.cur_direction =='W')                //checking for west direction
  {
      if( (cwall.right) and (micromouse.y < 15))
      {
        dec_to_bin(maze_wall_get(micromouse.x,(micromouse.y)+1));     //+1 for north neighbour
        if(bin[2] == 0)//south
          maze_wall("add",micromouse.x,(micromouse.y)+1,4);
    
      }
      if( (cwall.front) and (micromouse.x > 0))
      {
        dec_to_bin(maze_wall_get(micromouse.x-1,micromouse.y));     //+1 for north neighbour
        if(bin[1] == 0)
          maze_wall("add",micromouse.x-1,micromouse.y,2);
      }
      if( (cwall.left) and (micromouse.y >0))
      {
        dec_to_bin(maze_wall_get(micromouse.x,micromouse.y-1));    //+1 for east neighbour
  
       if(bin[4] == 0)//west
         maze_wall("add",micromouse.x,micromouse.y-1,16);
      }

      maze_wall("assign",micromouse.x,micromouse.y,1 + 8*(cwall.front) + 16*(cwall.right) + 4*(cwall.left));          //bit masking detected walls for current cube


      if(     (!(cwall.front))   and   (maze_wall_get(micromouse.x-1,micromouse.y)<cwt)  )
        micromouse.x-=1;
      else if(    (!(cwall.right))   and   (maze_wall_get(micromouse.x-1,micromouse.y+1)<cwt)  )
      {
          micromouse.y+=1;
          micromouse.cur_direction ='N';
      }
      else if(    (!(cwall.left))   and   (maze_wall_get(micromouse.x-1,micromouse.y-1)<cwt)  )
      {
          micromouse.y-=1;
          micromouse.cur_direction ='S';
      }


    else
     {
       add(micromouse.x,micromouse.y);    //add coordinates of current cube to stack
       minimum = 256; 
  
       while(check())                       //check whether stack is empty or not
       {
         
         i = 0;
         i = check();                         //take first non zero value from stack
  
         if(i == 0);
         {
           break;                         //if there is nothing in stack, then do nothing and run while loop again
         }
         x = stack[i-1][0];                    //assign coordinates of first value from stack to x and y 
         y = stack[i-1][1];
         if(state=="I")
         {
           if((x==7 || x==8)&& (y==7||y==8))
           {
            remove(x,y);
            continue;
           }
         }
         dec_to_bin(maze_wall_get(x,y));     //do bit demasking and assign binary number to n
  
         int a=0;
         int b=1;
         int c=0;
         for(i = 4; i>0; i--)                 //finding minimum values among the neighbours
         {
           if(bin[i] == 0)
           {
             if(maze_weight_get(x+a,y+b)<minimum)
               minimum = maze_weight_get(x+a,y+b);
               c++;//check if cell has all side walls 
           }
           a=a+b;
           b=a-b;
           a=a-b;
           if(i != 3)
           {
             a = -a;
             b = -b;
           }
         }
        if(c==0)
        {
          remove(x,y);
          continue;
        }
        if(minimum<max_wt)
        {
         if(maze_weight_get(x,y)-1 !=minimum)
         {
           maze_weight("assign",x,y,minimum+1);                // upgrade given cube's weight and ad all neighbours to stack
           if(x-1>0)
             add(x-1,y);
           if(x+1<16)
             add(x+1,y);
           if(y-1>0)
             add(x,y-1);
           if(y+1>0)
             add(x,y+1);
         }
        }
        remove(x,y);                                       //remove checked cube's coordinates from stack
      }
      
      int x1 = micromouse.x;
      int y1 = micromouse.y;
      
      
      if(     (!(cwall.front))   and   (maze_wall_get(micromouse.x-1,micromouse.y)<cwt)  )
        micromouse.x-=1;
      else if(    (!(cwall.right))   and   (maze_wall_get(micromouse.x-1,micromouse.y+1)<cwt)  )
      {
          micromouse.y+=1;
          micromouse.cur_direction ='N';
      }
      else if(    (!(cwall.left))   and   (maze_wall_get(micromouse.x-1,micromouse.y-1)<cwt)  )
      {
          micromouse.y-=1;
          micromouse.cur_direction ='S';
      }
      else
      {
         micromouse.x+=1;
         micromouse.cur_direction ='E';
      }
      
    }

  }




//=================================================================================
//                                   SOUTH
//===============================================================================





  if( micromouse.cur_direction =='S')                //checking for north direction
  {
     if( (cwall.front) and (micromouse.y >0))
     {


      //edit from here
       dec_to_bin(maze_wall_get(micromouse.x,micromouse.y-1));     //+1 for north neighbour
  
       if(bin[4] == 0)//south
         maze_wall("add",micromouse.x,(micromouse.y)-1,16);
     }
     if( (cwall.left) and (micromouse.x <15))
     {
       dec_to_bin(maze_wall_get(micromouse.x+1,micromouse.y));    //+1 for east neighbour
  
       if(bin[3] == 0)//west
         maze_wall("add",(micromouse.x)+1,micromouse.y,8);
     }
     if( (cwall.right) and (micromouse.x > 0))
     {
       dec_to_bin(maze_wall_get((micromouse.x)-1,micromouse.y));    //-1 for west neighbour
  
       if(bin[1] == 0)//east of neighbour cell
         maze_wall("add",(micromouse.x)-1,micromouse.y,2);
     } 
     maze_wall("assign",micromouse.x,micromouse.y,1 + 4*(cwall.front) + 8*(cwall.right) + 2*(cwall.left));          //bit masking detected walls for current cube
  
     
     if(     (!(cwall.front))   and   (maze_weight_get(micromouse.x,micromouse.y-1)<cwt)  )                      //preparing values for next cube
       micromouse.y-=1;
     else if(    (!(cwall.left))   and   (maze_weight_get((micromouse.x)+1,micromouse.y)<cwt)  )
     {
         micromouse.x+=1;
         micromouse.cur_direction ='E';
     }
     else if(    (!(cwall.right))   and   (maze_weight_get((micromouse.x)-1,micromouse.y)<cwt)  )
     {
         micromouse.x-=1;
         micromouse.cur_direction ='W';
     }
 
     else
     {
       add(micromouse.x,micromouse.y);    //add coordinates of current cube to stack
       minimum = 256; 
  
       while(check())                       //check whether stack is empty or not
       {
         
         i = 0;
         i = check();                         //take first non zero value from stack
  
         if(i == 0);
         {
           break;                         //if there is nothing in stack, then do nothing and run while loop again
         }
         x = stack[i-1][0];                    //assign coordinates of first value from stack to x and y 
         y = stack[i-1][1];
         if(state=="I")
         {
           if((x==7 || x==8)&& (y==7||y==8))
           {
            remove(x,y);
            continue;
           }
         }
         dec_to_bin(maze_wall_get(x,y));     //do bit demasking and assign binary number to n
  
         int a=0;
         int b=1;
         int c=0;
         for(i = 4; i>0; i--)                 //finding minimum values among the neighbours
         {
           if(bin[i] == 0)
           {
             if(maze_weight_get(x+a,y+b)<minimum)
               minimum = maze_weight_get(x+a,y+b);
               c++;
           }
           a=a+b;
           b=a-b;
           a=a-b;
           if(i != 3)
           {
             a = -a;
             b = -b;
           }
         }
        if(c==0)
        {
          remove(x,y);
          continue;
        }
        if(minimum<max_wt)
        {
         if(maze_weight_get(x,y)-1 !=minimum)
         {
           maze_weight("assign",x,y,minimum+1);                // upgrade given cube's weight and ad all neighbours to stack
           if(x-1>0)
             add(x-1,y);
           if(x+1<16)
             add(x+1,y);
           if(y-1>0)
             add(x,y-1);
           if(y+1>0)
             add(x,y+1);
         }
        }
        remove(x,y);                                       //remove checked cube's coordinates from stack
      }
      int x1 = micromouse.x;
      int y1 = micromouse.y;
      if(     (!(cwall.front))   and   (maze_weight_get(micromouse.x,micromouse.y-1)<cwt)  )                      //preparing values for next cube
       micromouse.y-=1;
     else if(    (!(cwall.left))   and   (maze_weight_get((micromouse.x)+1,micromouse.y)<cwt)  )
     {
         micromouse.x+=1;
         micromouse.cur_direction ='E';
     }
     else if(    (!(cwall.right))   and   (maze_weight_get((micromouse.x)-1,micromouse.y)<cwt)  )
     {
         micromouse.x-=1;
         micromouse.cur_direction ='W';
     }
      else
      {
         micromouse.y+=1;
         micromouse.cur_direction ='N';
      }
    }    
  }

  
}


void movement() 
{
  change_direction(micromouse.prev_direction,micromouse.cur_direction);
  move_next_cell();
}

void move_next_cell()
{
  leftrot=0;
  rightrot=0;
  ultrasonic();
  walls();
  while(leftrot<2 and rightrot<2)
  { Serial.println(leftrot);
  Serial.println(leftrot);
    if(leftwall)
      {
        leftfollow();
      }
    else if(rightwall)
      {
        rightfollow();
      }
  }
}
/*===========================================================================================================================================
 *=========================================================================================================================================== 
 

 *=========================================================================================================================================
 ==========================================================================================================================================
 */


void motors()
{
  forward();
  delay(1000);
  stop();
  delay(1000);
  backward();
  delay(1000);
  stop();
  delay(1000);
}
void forward()
{
  digitalWrite(ml2,HIGH);
  digitalWrite(ml1,LOW);
  digitalWrite(mr2,HIGH);
  digitalWrite(mr1,LOW);
}
void backward()
{
  digitalWrite(ml1,HIGH);
  digitalWrite(ml2,LOW);
  digitalWrite(mr1,HIGH);
  digitalWrite(mr2,LOW);
}
void stop()
{
  digitalWrite(ml1,LOW);
  digitalWrite(ml2,LOW);
  digitalWrite(mr1,LOW);
  digitalWrite(mr2,LOW);
}
//void right()
//{
//  digitalWrite(ml2,HIGH);
//  digitalWrite(ml1,LOW);
//  digitalWrite(mr2,LOW);
//  digitalWrite(mr1,HIGH);
//}
//void soft_right()
//{
//  digitalWrite(ml2,HIGH);
//  digitalWrite(ml1,LOW);
//  digitalWrite(mr2,LOW);
//  digitalWrite(mr1,LOW);
//}
//void left()
//{
//  digitalWrite(ml2,LOW);
//  digitalWrite(ml1,HIGH);
//  digitalWrite(mr2,HIGH);
//  digitalWrite(mr1,LOW);
//}
//void soft_left()
//{
//  digitalWrite(ml2,LOW);
//  digitalWrite(ml1,LOW);
//  digitalWrite(mr2,HIGH);
//  digitalWrite(mr1,LOW);
//}
void left()
{ 
  
  analogWrite(enr,100);
analogWrite(enl,100);
//  right();

 analogWrite(enr,100);
analogWrite(enl,100);
  leftrot=0;
  rightrot=0;
  while(leftrot<1 and rightrot<1)
  { Serial.println(leftrot);
    left();
//    delay(10);
//    stop();
//    delay(2);
  }

}
void right()
{
     analogWrite(enr,100);
analogWrite(enl,100);
  left();
   analogWrite(enr,100);
  analogWrite(enl,100);
  leftrot=0;
  rightrot=0;
  while( leftrot<1 and rightrot<1)
  {
    Serial.println(leftrot);
    right();
  }

}
void u_turn()
{
  analogWrite(enr,100);
analogWrite(enl,100);
  right();
  delay(5);
   analogWrite(enr,100);
analogWrite(enl,100);
 leftrot=0;
  rightrot=0;
  while(leftrot<2 and rightrot<2)   
  {
    Serial.println(leftrot);
    left();
  }
}
void encoder()
{
  Serial.println(leftrot);
  Serial.println(rightrot);
  
}


//void ultrasonic()
//{ 
//  int duration = sonar_l.ping();
//  dis_left = (duration / 2) * 0.343;
//  duration = sonar_m.ping();
//  dis_middle = (duration / 2) * 0.343;
//  duration = sonar_r.ping();
//  dis_right = (duration / 2) * 0.343;
//  
//  Serial.println("Distances in CM: ");
//  Serial.println(dis_left );
//  Serial.println(dis_middle);
//  Serial.println(dis_right);
//}
void ultrasonic()
{

  int duration = sonar_l.ping();
 
  dis_left = (duration / 2) * 0.343;
  duration = sonar_m.ping();
  dis_middle = (duration / 2) * 0.343;
  duration = sonar_r.ping();
  dis_right = (duration / 2) * 0.343;

  leftSensor = dis_left;
  rightSensor = dis_right;
  frontSensor = dis_middle;

  oldLeftSensor = leftSensor; // save old readings for movment
  oldRightSensor = rightSensor;
  oldFrontSensor = frontSensor;
  Serial.print("Distances in MM: ");
  Serial.println(dis_left   );
  Serial.println(dis_middle   );
  Serial.println(dis_right);
}
void walls() {


  if (leftSensor < wall_threshold ) {
    leftwall = true ;
  }
  else{
    leftwall = false ;
  }


  if( rightSensor < wall_threshold ) {
    rightwall = true ;
  }
  else {
    rightwall = false ;


  } if ( frontSensor < front_threshold ) {
    frontwall = true ;
  }
  else {
    frontwall = false ;
  }

}
void bothfollow()
{
  leftrot=0;
rightrot=0;
 ultrasonic();
 errorP=rightSensor-leftSensor;
 totalError=0.2*errorP;
 analogWrite(enr,BaseSpeed - totalError);
 analogWrite(enl,BaseSpeed + totalError);
  Serial.print("totalErrr");
   Serial.println(totalError);
 forward();
 delay(5);
  
}
void leftfollow()
{
leftrot=0;
rightrot=0;
 ultrasonic();
 errorP=38-leftSensor;
 errorD = (errorP - oldErrorP);
errorE=(rightrot-leftrot)*3;
 oldErrorP=errorP;


 totalError=0.3*errorP-0*errorE;
 Serial.print("err:");
 Serial.println(errorP);
 Serial.print(errorD);
 Serial.print(totalError);
 analogWrite(enr,BaseSpeed - totalError);
 analogWrite(enl,BaseSpeed + totalError);
 
 forward();
 delay(5);
 if(leftSensor>100)
 {stop();
 delay(5000);}

}
void rightfollow()
{
 ultrasonic();

 errorP=40-rightSensor;
 oldErrorP=errorP;
 errorD = errorP - oldErrorP;
 totalError=0.3*errorP+0.1*errorD;
 
 analogWrite(enr,BaseSpeed + totalError);
 analogWrite(enl,BaseSpeed - totalError);
  Serial.print("totalErrr");
   Serial.println(totalError);
 forward();
 delay(5);
}

/*--------------------------------------------------------------------------------------------------------------------------
 *                     algo    part
 ---------------------------------------------------------------------------------------------------------------------------*/
void change_direction(char prev,char next)
{
  if(prev != next)
  {
    if (prev=='N')
    {
      if (next=='S')
         u_turn();
      else if (next == 'E')
        right();
      else if (next == 'W')
        left();
    }
    else if (prev=='E')
    {
      if (next=='W')
         u_turn();
      else if (next == 'S')
        right();
      else if (next == 'N')
        left();
    }
    if (prev=='W')
    {
      if (next=='E')
         u_turn();
      else if (next == 'N')
        right();
      else if (next == 'S')
        left();
    }
    if (prev=='S')
    {
      if (next=='N')
         u_turn();
      else if (next == 'W')
        right();
      else if (next == 'E')
        left();
    }  
  }
}
