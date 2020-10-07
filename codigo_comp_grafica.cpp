#define SDL_MAIN_HANDLED
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <stack>
#include <sstream>
#include <unistd.h>
#include <time.h>

using namespace std;

// Estrutura para representar pontos
typedef struct {int x, y;} Point;

typedef struct {Point *pts;} Polygon;

// Vari�veis necess�rias para o SDL
unsigned int * pixels;
int width, height;
SDL_Surface * window_surface;
SDL_Renderer * renderer;

// T�tulo da janela
std::string titulo = "SDL Random Points";

// Valores RGB para a cor de funco da janela
const int VERMELHO = 0;
const int VERDE = 0;
const int AZUL = 0;

const int CENTRO_X = 400;
const int CENTRO_Y = 300;

const int LIMITE = 275;



// Gera uma estrutura Point a partir de valores para x e y
Point getPoint(int x, int y)
{
    Point p;
    p.x = x;
    p.y = y;
    return p;
}

// Obt�m a cor de um pixel de uma determinada posi��o
Uint32 getPixel(int x, int y)
{
    if((x>=0 && x<=width) && (y>=0 && y<=height))
        return pixels[x + width * y];
    else
        return -1;
}

// Seta um pixel em uma determinada posi��o,
// atrav�s da coordenadas de cor r, g, b, e alpha (transpar�ncia)
// r, g, b e a variam de 0 at� 255
void setPixel(int x, int y, int r, int g, int b, int a)
{
    pixels[x + y * width] = SDL_MapRGBA(window_surface->format, r, g, b, a);
}

// Seta um pixel em uma determinada posi��o,
// atrav�s da coordenadas de cor r, g e b
// r, g, e b variam de 0 at� 255
void setPixel(int x, int y, int r, int g, int b)
{
    setPixel(x, y, r, g, b, 255);
}

// Mostra na barra de t�tulo da janela a posi��o
// corrente do mouse
void showMousePosition(SDL_Window * window, int x, int y)
{
    std::stringstream ss;
    ss << titulo << " X: " << x << " Y: " << y;
    SDL_SetWindowTitle(window, ss.str().c_str());
}

// Imprime na console a posi��o corrente do mouse
void printMousePosition(int x, int y)
{
    printf("Mouse on x = %d, y = %d\n",x,y);
}

// Seta um pixel em uma determinada posi��o,
// atrav�s de um Uint32 representando
// uma cor RGB
void setPixel(int x, int y, Uint32 color)
{
    if((x<0 || x>=width || y<0 || y>=height)) {
        printf("Coordenada inv�lida : (%d,%d)\n",x,y);
        return;
    }
    pixels[x + y * width] = color;
}

// Retorna uma cor RGB(UInt32) formada
// pelas componentes r, g, b e a(transpar�ncia)
// informadas. r, g, b e a variam de 0 at� 255
Uint32 RGB(int r, int g, int b, int a) {
    return SDL_MapRGBA(window_surface->format, r, g, b, a);
}

// Retorna uma cor RGB(UInt32) formada
// pelas componentes r, g, e b
// informadas. r, g e b variam de 0 at� 255
// a transpar�ncia � sempre 255 (imagem opaca)
Uint32 RGB(int r, int g, int b) {
    return SDL_MapRGBA(window_surface->format, r, g, b, 255);
}

// Retorna um componente de cor de uma cor RGB informada
// aceita os par�metros 'r', 'R','g', 'G','b' e 'B',
Uint8 getColorComponent( Uint32 pixel, char component ) {

    Uint32 mask;

    switch(component) {
        case 'b' :
        case 'B' :
            mask = RGB(0,0,255);
            pixel = pixel & mask;
            break;
        case 'r' :
        case 'R' :
            mask = RGB(255,0,0);
            pixel = pixel & mask;
            pixel = pixel >> 16;
            break;
        case 'g' :
        case 'G' :
            mask = RGB(0,255,0);
            pixel = pixel & mask;
            pixel = pixel >> 8;
            break;
    }
    return (Uint8) pixel;
}
//--------- ALGORITMO DE DESENHO DE RETAS ----------
// PARAMS (X0 E YO = PONTO INICIAL)
//        (X1 E Y1 = PONTO FINAL)
void drawWuLine(int x0, int y0, int x1, int y1, Uint32 clrLine )
{
    /* Make sure the line runs top to bottom */
    if (y0 > y1)
    {
        int aux = y0; y0 = y1; y1 = aux;
        aux = x0; x0 = x1; x1 = aux;
    }

    /* Draw the initial pixel, which is always exactly intersected by
    the line and so needs no weighting */
    setPixel( x0, y0, clrLine );

    int xDir, deltaX = x1 - x0;
    if( deltaX >= 0 )
    {
        xDir = 1;
    }
    else
    {
        xDir   = -1;
        deltaX = 0 - deltaX; /* make deltaX positive */
    }

    /* Special-case horizontal, vertical, and diagonal lines, which
    require no weighting because they go right through the center of
    every pixel */
    int deltaY = y1 - y0;
    if (deltaY == 0)
    {
        /* Horizontal line */
        while (deltaX-- != 0)
        {
            x0 += xDir;
            setPixel( x0, y0, clrLine );
        }
        return;
    }
    if (deltaX == 0)
    {
        /* Vertical line */
        do
        {
            y0++;
            setPixel( x0, y0, clrLine );
        } while (--deltaY != 0);
        return;
    }

    if (deltaX == deltaY)
    {
        /* Diagonal line */
        do
        {
            x0 += xDir;
            y0++;
            setPixel( x0, y0, clrLine );
        } while (--deltaY != 0);
        return;
    }

    unsigned short errorAdj;
    unsigned short errorAccaux, weighting;

    /* Line is not horizontal, diagonal, or vertical */
    unsigned short errorAcc = 0;  /* initialize the line error accumulator to 0 */

    Uint32 rl = getColorComponent( clrLine, 'r' );
    Uint32 gl = getColorComponent( clrLine, 'g' );
    Uint32 bl = getColorComponent( clrLine, 'b' );
    double grayl = rl * 0.299 + gl * 0.587 + bl * 0.114;

    /* Is this an X-major or Y-major line? */
    if (deltaY > deltaX)
    {
    /* Y-major line; calculate 16-bit fixed-point fractional part of a
    pixel that X advances each time Y advances 1 pixel, truncating the
        result so that we won't overrun the endpoint along the X axis */
        errorAdj = ((unsigned long) deltaX << 16) / (unsigned long) deltaY;
        /* Draw all pixels other than the first and last */
        while (--deltaY) {
            errorAccaux = errorAcc;   /* remember currrent accumulated error */
            errorAcc += errorAdj;      /* calculate error for next pixel */
            if (errorAcc <= errorAccaux) {
                /* The error accumulator turned over, so advance the X coord */
                x0 += xDir;
            }
            y0++; /* Y-major, so always advance Y */
                  /* The IntensityBits most significant bits of errorAcc give us the
                  intensity weighting for this pixel, and the complement of the
            weighting for the paired pixel */
            weighting = errorAcc >> 8;
            /*
            ASSERT( weighting < 256 );
            ASSERT( ( weighting ^ 255 ) < 256 );
            */
            Uint32 clrBackGround = getPixel(x0, y0 );
            Uint8 rb = getColorComponent( clrBackGround, 'r' );
            Uint8 gb = getColorComponent( clrBackGround, 'g' );
            Uint8 bb = getColorComponent( clrBackGround, 'b' );
            double grayb = rb * 0.299 + gb * 0.587 + bb * 0.114;

            Uint8 rr = ( rb > rl ? ( ( Uint8 )( ( ( double )( grayl<grayb?weighting:(weighting ^ 255)) ) / 255.0 * ( rb - rl ) + rl ) ) : ( ( Uint8 )( ( ( double )( grayl<grayb?weighting:(weighting ^ 255)) ) / 255.0 * ( rl - rb ) + rb ) ) );
            Uint8 gr = ( gb > gl ? ( ( Uint8 )( ( ( double )( grayl<grayb?weighting:(weighting ^ 255)) ) / 255.0 * ( gb - gl ) + gl ) ) : ( ( Uint8 )( ( ( double )( grayl<grayb?weighting:(weighting ^ 255)) ) / 255.0 * ( gl - gb ) + gb ) ) );
            Uint8 br = ( bb > bl ? ( ( Uint8 )( ( ( double )( grayl<grayb?weighting:(weighting ^ 255)) ) / 255.0 * ( bb - bl ) + bl ) ) : ( ( Uint8 )( ( ( double )( grayl<grayb?weighting:(weighting ^ 255)) ) / 255.0 * ( bl - bb ) + bb ) ) );
            setPixel( x0, y0, RGB( rr, gr, br ) );

            clrBackGround = getPixel(x0 + xDir, y0 );
            rb = getColorComponent( clrBackGround, 'r' );
            gb = getColorComponent( clrBackGround, 'g' );
            bb = getColorComponent( clrBackGround, 'b' );
            grayb = rb * 0.299 + gb * 0.587 + bb * 0.114;

            rr = ( rb > rl ? ( ( Uint8 )( ( ( double )( grayl<grayb?(weighting ^ 255):weighting) ) / 255.0 * ( rb - rl ) + rl ) ) : ( ( Uint8 )( ( ( double )( grayl<grayb?(weighting ^ 255):weighting) ) / 255.0 * ( rl - rb ) + rb ) ) );
            gr = ( gb > gl ? ( ( Uint8 )( ( ( double )( grayl<grayb?(weighting ^ 255):weighting) ) / 255.0 * ( gb - gl ) + gl ) ) : ( ( Uint8 )( ( ( double )( grayl<grayb?(weighting ^ 255):weighting) ) / 255.0 * ( gl - gb ) + gb ) ) );
            br = ( bb > bl ? ( ( Uint8 )( ( ( double )( grayl<grayb?(weighting ^ 255):weighting) ) / 255.0 * ( bb - bl ) + bl ) ) : ( ( Uint8 )( ( ( double )( grayl<grayb?(weighting ^ 255):weighting) ) / 255.0 * ( bl - bb ) + bb ) ) );
            setPixel( x0 + xDir, y0, RGB( rr, gr, br ) );
        }
        /* Draw the final pixel, which is always exactly intersected by the line
        and so needs no weighting */
        setPixel( x1, y1, clrLine );
        return;
    }
    /* It's an X-major line; calculate 16-bit fixed-point fractional part of a
    pixel that Y advances each time X advances 1 pixel, truncating the
    result to avoid overrunning the endpoint along the X axis */
    errorAdj = ((unsigned long) deltaY << 16) / (unsigned long) deltaX;
    /* Draw all pixels other than the first and last */
    while (--deltaX) {
        errorAccaux = errorAcc;   /* remember currrent accumulated error */
        errorAcc += errorAdj;      /* calculate error for next pixel */
        if (errorAcc <= errorAccaux) {
            /* The error accumulator turned over, so advance the Y coord */
            y0++;
        }
        x0 += xDir; /* X-major, so always advance X */
                    /* The IntensityBits most significant bits of errorAcc give us the
                    intensity weighting for this pixel, and the complement of the
        weighting for the paired pixel */
        weighting = errorAcc >> 8;
        /*
        ASSERT( weighting < 256 );
        ASSERT( ( weighting ^ 255 ) < 256 );
        */
        Uint32 clrBackGround = getPixel(x0, y0 );
        Uint8 rb = getColorComponent( clrBackGround, 'r' );
        Uint8 gb = getColorComponent( clrBackGround, 'g' );
        Uint8 bb = getColorComponent( clrBackGround, 'b' );
        double grayb = rb * 0.299 + gb * 0.587 + bb * 0.114;

        Uint8 rr = ( rb > rl ? ( ( Uint8 )( ( ( double )( grayl<grayb?weighting:(weighting ^ 255)) ) / 255.0 * ( rb - rl ) + rl ) ) : ( ( Uint8 )( ( ( double )( grayl<grayb?weighting:(weighting ^ 255)) ) / 255.0 * ( rl - rb ) + rb ) ) );
        Uint8 gr = ( gb > gl ? ( ( Uint8 )( ( ( double )( grayl<grayb?weighting:(weighting ^ 255)) ) / 255.0 * ( gb - gl ) + gl ) ) : ( ( Uint8 )( ( ( double )( grayl<grayb?weighting:(weighting ^ 255)) ) / 255.0 * ( gl - gb ) + gb ) ) );
        Uint8 br = ( bb > bl ? ( ( Uint8 )( ( ( double )( grayl<grayb?weighting:(weighting ^ 255)) ) / 255.0 * ( bb - bl ) + bl ) ) : ( ( Uint8 )( ( ( double )( grayl<grayb?weighting:(weighting ^ 255)) ) / 255.0 * ( bl - bb ) + bb ) ) );

        setPixel( x0, y0, RGB( rr, gr, br ) );

        clrBackGround = getPixel(x0, y0 + 1 );
        rb = getColorComponent( clrBackGround, 'r' );
        gb = getColorComponent( clrBackGround, 'g' );
        bb = getColorComponent( clrBackGround, 'b' );
        grayb = rb * 0.299 + gb * 0.587 + bb * 0.114;

        rr = ( rb > rl ? ( ( Uint8 )( ( ( double )( grayl<grayb?(weighting ^ 255):weighting) ) / 255.0 * ( rb - rl ) + rl ) ) : ( ( Uint8 )( ( ( double )( grayl<grayb?(weighting ^ 255):weighting) ) / 255.0 * ( rl - rb ) + rb ) ) );
        gr = ( gb > gl ? ( ( Uint8 )( ( ( double )( grayl<grayb?(weighting ^ 255):weighting) ) / 255.0 * ( gb - gl ) + gl ) ) : ( ( Uint8 )( ( ( double )( grayl<grayb?(weighting ^ 255):weighting) ) / 255.0 * ( gl - gb ) + gb ) ) );
        br = ( bb > bl ? ( ( Uint8 )( ( ( double )( grayl<grayb?(weighting ^ 255):weighting) ) / 255.0 * ( bb - bl ) + bl ) ) : ( ( Uint8 )( ( ( double )( grayl<grayb?(weighting ^ 255):weighting) ) / 255.0 * ( bl - bb ) + bb ) ) );

        setPixel( x0, y0 + 1, RGB( rr, gr, br ) );
    }

    /* Draw the final pixel, which is always exactly intersected by the line
    and so needs no weighting */
    setPixel( x1, y1, clrLine );
}
//--------- DESENHA UM POLIGONO ----------

void desenhaPlg(Point p[], int qtd, Uint32 color){

    for(int i=0; i<qtd;i++){
        Point pi = p[i];
        Point pf;

        if(i==qtd-1)
        pf = p[0];
        else
        pf = p[i+1];

        drawWuLine(pi.x,pi.y,pf.x,pf.y,color);

    }

}

void translad(Point p[],int qtd,int transX,int transY){

    for(int i = 0; i < qtd; i++){
       p[i].x += transX;
       p[i].y += transY;
    }

}
/*------------------------RECURSOS-----------------------*/
// //Aqui esta sendo desenhado a moldura do grafico e a grade

void base_grafico(){

    drawWuLine(675,575,675,25,RGB(66,66,66));
    drawWuLine(675,25,125,25,RGB(66,66,66));
    
    //CENTRO
    drawWuLine(400,25,400,575,RGB(66,66,66));
    drawWuLine(125,300,675,300,RGB(66,66,66));

    //ESQUERDA Vertical
    drawWuLine(262.5,25,262.5,575,RGB(66,66,66));

    //DIREITA Vertical
    drawWuLine(537.5,25,537.5,575,RGB(66,66,66));

    //CIMA Horizontal
    drawWuLine(125,162.5,675,162.5,RGB(66,66,66));

    //BAIXO Horizontal
    drawWuLine(125,437.5,675,437.5,RGB(66,66,66));

    //GRAFICO
        //LINHA VERTICAL
    drawWuLine(125,25,125,575,RGB(255,255,255));

        //LINHA HORIZONTAL
    drawWuLine(125,575,675,575,RGB(255,255,255));

        //MARCAS HORIZONTAIS
    drawWuLine(115,25,135,25,RGB(255,255,255));
    drawWuLine(115,162.5,135,162.5,RGB(255,255,255));
    drawWuLine(115,300,135,300,RGB(255,255,255));
    drawWuLine(115,437.5,135,437.5,RGB(255,255,255));

        //MARCAS HORIZONTAIS
    drawWuLine(262.5,565,262.5,585,RGB(255,255,255));
    drawWuLine(400,565,400,585,RGB(255,255,255));
    drawWuLine(537.5,565,537.5,585,RGB(255,255,255));
    drawWuLine(675,565,675,585,RGB(255,255,255));
}

/*------------------------EQUA��ES----------------------*/
//Aqui est�o as equa��es

/* Cada fun��o ja esta configurada pra desenhar a equa��o no grafico
 * passando a varia��o do eixo X e do eixo Y
 * exemplo:
 *     equa��o 1 varie no eixo X de -5 at� 5
 *     equa��o 1 varie no eixo Y de -1 at� 5
 *
 *     eq1(-5,5,-1,5);
 */

void eq1(int minimoX, int maximoX,int minimoY, int maximoY){

   for(float i = minimoX ; i <= maximoX ; i+=0.03){
    float x = i;
    float y = x;
    float xd,yd;

    xd = (((x - (minimoX)) * ((CENTRO_X + LIMITE - 1) - (CENTRO_X - LIMITE + 1))) / (maximoX - (minimoX)) ) + (CENTRO_X - LIMITE + 1);
    yd = (((y - (minimoY)) * ((CENTRO_Y - LIMITE + 1)-(CENTRO_Y + LIMITE - 1))) / (maximoY - (minimoY)) ) + (CENTRO_Y + LIMITE - 1);

    setPixel(xd,yd,RGB(255,0,255));
   }


}

void eq2(int minimoX, int maximoX,int minimoY, int maximoY){

   for(float i = minimoX ; i <= maximoX ; i+=0.03){
        float x = i;
        float y = -x;
        float xd,yd;

        xd = (((x - (minimoX)) * ((CENTRO_X + LIMITE - 1) - (CENTRO_X - LIMITE + 1))) / (maximoX - (minimoX)) ) + (CENTRO_X - LIMITE + 1);
        yd = (((y - (minimoY)) * ((CENTRO_Y - LIMITE + 1)-(CENTRO_Y + LIMITE - 1))) / (maximoY - (minimoY)) ) + (CENTRO_Y + LIMITE - 1);

        setPixel(xd,yd,RGB(255,0,0));
    }

}
void eq3(int minimoX, int maximoX,int minimoY, int maximoY){

   for(float i = minimoX ; i <= maximoX ; i+=0.03){
        float x = i;
        float y =  (2*x*x)-(6*x) + 1;
        float xd,yd;

        xd = (((x - (minimoX)) * ((CENTRO_X + LIMITE - 1) - (CENTRO_X - LIMITE + 1))) / (maximoX - (minimoX)) ) + (CENTRO_X - LIMITE + 1);
        yd = (((y - (minimoY)) * ((CENTRO_Y - LIMITE + 1)-(CENTRO_Y + LIMITE - 1))) / (maximoY - (minimoY)) ) + (CENTRO_Y + LIMITE - 1);

        if(yd > CENTRO_Y - LIMITE && yd < CENTRO_Y + LIMITE)
        setPixel(xd,yd,RGB(0,255,0));
    }

}
void eq4(int minimoX, int maximoX,int minimoY, int maximoY){

   for(float i = minimoX ; i <= maximoX ; i+=0.03){
        float x = i;
        float y =  sin(x);
        float xd,yd;

        xd = (((x - (minimoX)) * ((CENTRO_X + LIMITE - 1) - (CENTRO_X - LIMITE + 1))) / (maximoX - (minimoX)) ) + (CENTRO_X - LIMITE + 1);
        yd = (((y - (minimoY)) * ((CENTRO_Y - LIMITE + 1)-(CENTRO_Y + LIMITE - 1))) / (maximoY - (minimoY)) ) + (CENTRO_Y + LIMITE - 1);

        if(yd > CENTRO_Y - LIMITE && yd < CENTRO_Y + LIMITE)
        setPixel(xd,yd,RGB(255,255,0));
    }

}
void eq5(int minimoX, int maximoX,int minimoY, int maximoY){

   for(float i = minimoX ; i <= maximoX ; i+=0.03){
        float x = i;
        float y =  -(x*x)+4;
        float xd,yd;

        xd = (((x - (minimoX)) * ((CENTRO_X + LIMITE - 1) - (CENTRO_X - LIMITE + 1))) / (maximoX - (minimoX)) ) + (CENTRO_X - LIMITE + 1);
        yd = (((y - (minimoY)) * ((CENTRO_Y - LIMITE + 1)-(CENTRO_Y + LIMITE - 1))) / (maximoY - (minimoY)) ) + (CENTRO_Y + LIMITE - 1);

        if(yd > CENTRO_Y - LIMITE && yd < CENTRO_Y + LIMITE)
        setPixel(xd,yd,RGB(100,100,255));
    }

}

// Inicialmente todas est�o de -10 ate 10 em ambos os eixos

//Para fazer as fun��es das teclas, � s� variar esses parametros

//DISPLAY � a fun��o que mostra as coisas na tela, sendo chamada na MAIN
void display(int x1, int x2, int y1, int y2)
{
    base_grafico();
    eq1(x1, x2, y1, y2);
    eq2(x1, x2, y1, y2);
    eq3(x1, x2, y1, y2);
    eq4(x1, x2, y1, y2);
    eq5(x1, x2, y1, y2);
}

// Inicializa o SDL, abre a janela e controla o loop
// principal do controle de eventos
int main()
{
    // Inicializa��es iniciais obrigat�rias
    srand (time(NULL));
    setlocale(LC_ALL, NULL);

    int x1 = -10;
    int y1 = -10;
    int x2 = 10;
    int y2 = 10;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window * window = SDL_CreateWindow(titulo.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_RESIZABLE);

    window_surface = SDL_GetWindowSurface(window);

    pixels = (unsigned int *) window_surface->pixels;
    width = window_surface->w;
    height = window_surface->h;

    // Fim das inicializa��es

    printf("Pixel format: %s\n",
        SDL_GetPixelFormatName(window_surface->format->format));

     while (1)
    {

        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                exit(0);
            }

            // Keypress
            if (event.type == SDL_KEYDOWN){
                switch( event.key.keysym.sym ){

                    case SDLK_MINUS:
                        x1--;
                        x2++;
                        y1--;
                        y2++;
                        printf("Pressionada a tecla -\n");
                        break;
                    
                    case SDLK_KP_MINUS:
                        x1--;
                        x2++;
                        y1--;
                        y2++;
                        printf("Pressionada a tecla -\n");
                        break;
                    case SDLK_PLUS:
                        x1++;
                        x2--;
                        y1++;
                        y2--;
                        printf("Pressionada a tecla +\n");
                        break;
                    case SDLK_KP_PLUS:
                        x1++;
                        x2--;
                        y1++;
                        y2--;
                        printf("Pressionada a tecla +\n");
                        break;
                    case SDLK_LEFT:
                        x1--;
                        x2--;
                        printf("Pressionada a tecla LEFT\n");
                        break;
                    case SDLK_RIGHT:
                        x1++;
                        x2++;
                        printf("Pressionada a tecla RIGHT\n");
                        break;
                    case SDLK_UP:
                        y1++;
                        y2++;
                        printf("Pressionada a tecla UP\n");
                        break;
                    case SDLK_DOWN:
                        y1--;
                        y2--;
                        printf("Pressionada a tecla DOWN\n");
                        break;
                    default:
                        break;
                }
            }


            if (event.type == SDL_WINDOWEVENT)
            {
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    window_surface = SDL_GetWindowSurface(window);
                    pixels = (unsigned int *) window_surface->pixels;
                    width = window_surface->w;
                    height = window_surface->h;
                    printf("Size changed: %d, %d\n", width, height);
                }
            }

            // Se o mouse � movimentado
            if(event.type == SDL_MOUSEMOTION)
            {
                // Mostra as posi��es x e y do mouse
                showMousePosition(window,event.motion.x,event.motion.y);
            }
            if(event.type == SDL_MOUSEBUTTONDOWN)
            {
				/*Se o bot�o esquerdo do mouse � pressionado */
                if(event.button.button == SDL_BUTTON_LEFT)
				{
					printf("Mouse pressed on (%d,%d)\n",event.motion.x,event.motion.y) ;
				}
            }
        }

        // Seta a cor de fundo da janela para a informada nas
        // constantes VERMELHO, VERDE e AZUL
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                setPixel(x, y, RGB(VERMELHO,VERDE,AZUL));
            }
        }

        display(x1, x2, y1, y2);

        SDL_UpdateWindowSurface(window);
    }
}
