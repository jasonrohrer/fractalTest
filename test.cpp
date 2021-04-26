#include "minorGems/graphics/Image.h"
#include "minorGems/graphics/converters/TGAImageConverter.h"
#include "minorGems/io/file/File.h"
#include "minorGems/io/file/FileOutputStream.h"
#include "minorGems/util/random/CustomRandomSource.h"

CustomRandomSource randSource( 3454 );

    
int maxIter = 200;

int emptyIterVal = 1024 * 1024;

char renderIterMap = true;


void clearIterPoints( double *iterXPoints, double *iterYPoints ) {
    if( ! renderIterMap ) return;
    
    for( int i=0; i<maxIter; i++ ) {
        iterXPoints[i] = emptyIterVal;
        iterYPoints[i] = emptyIterVal;
        }
    }



int main( int inNumArgs, char *inArgs[] ) {
    
    double zoomFactor = 100;
    
    if( inNumArgs > 1 ) {
        sscanf( inArgs[1], "%lf", &zoomFactor );
        }
    
    

    int w = 1000;
    int h = 1000;

    Image im( w, h, 3 );


    double maxTraceCount = 0;
    
    double **traceCount = new double*[h];
    
    for( int y=0; y<h; y++ ) {
        traceCount[y] = new double[w];
        
        for( int x=0; x<w; x++ ) {
            traceCount[y][x] = 0;
            }
        }
    
    Color inColor( 0, 0, 0 );

    double *iterXPoints = new double[ maxIter ];
    double *iterYPoints = new double[ maxIter ];
    

    // random colors for bands outside set
    Color *outColors = new Color[ maxIter ];
    
    for( int i=0; i<maxIter; i++ ) {
        Color *c = Color::makeColorFromHSV( 
            randSource.getRandomBoundedDouble( 0, 1.0 ),
            1, 1 );
        
        outColors[i] = *c;
        }
    
    // now smooth the colors together with neighbors
    for( int t=0; t<2; t++ )
    for( int i=1; i<maxIter-1; i++ ) {
        Color *firstBlend = 
            Color::linearSum( & outColors[i], & outColors[ i - 1 ], 0.66 );
        
        Color *secondBlend = 
            Color::linearSum( firstBlend, & outColors[ i + 1 ], 0.66 );

        outColors[i] = *firstBlend;
        delete firstBlend;
        delete secondBlend;
        }
    

    Color outColor( 1, 1, 1 );
    

    //double offsetX = -1.24254013716898265806;
    //double offsetY = 0.413238151606368892027;
    
    double offsetX = 0;
    double offsetY = 0;
    
    double maxDist = 0;
    
    // controls what counts as an "escaped" value
    double B = 256;

    
    for( int y=0; y<h; y++ ) {
        for( int x=0; x<w; x++ ) {
            
            double y0 = ( y - h / 2 ) / zoomFactor - offsetY;
            double x0 = ( x - w / 2 ) / zoomFactor + offsetX;
            
            double xA = x0;
            double yA = y0;
            
            // julia set
            // fix x0, y0
            //x0 = .552;
            //y0 = .552;
            
            double xAOld = xA;
            double yAOld = yA;
            

            int i = 0;
            clearIterPoints( iterXPoints, iterYPoints );
            
            while(  xA * xA + yA * yA < B * B &&
                   i < maxIter ) {
                
                // Mandlebrot
                double xB = xA * xA  - yA * yA + x0;
                double yB = 2* xA * yA + y0;
                
                
                // use two sets of previous values
                //double xB = xA * xA - yAOld * yAOld + x0;
                //double yB = xAOld * yA  + xAOld * yA + y0;

                //double xB = pow( xA, x0 )  - pow( yA, y0 ) + x0;
                //double yB = 2* xA * yA + y0;
                
                // cube mandelbrot
                //double xB = xA * xA * xA - 3 * xA * yA * yA + x0;
                //double yB = 3 * xA * xA * yA - yA * yA * yA + y0;
                
                xAOld = xA;
                yAOld = yA;
                xA = xB;
                yA = yB;

                iterXPoints[i] = xB;
                iterYPoints[i] = yB;                

                i++;                
                }
            
            if( i < maxIter ) {
                if( i < maxIter - 1 ) {
                    // smooth coloring algorithm
                    double outDist = sqrt( xA * xA + yA * yA );
                    
                    Color *blend = 
                        Color::linearSum( &outColors[i],
                                          &outColors[i+1],
                                          log( log( outDist ) / log( B ) ) /
                                          log( 2 ) );
                    im.setColor( y * w + x, *blend );
                    delete [] blend;
                    }
                else {
                    im.setColor( y * w + x, outColors[i] );
                    }
                
                // add the trace points in
                if( renderIterMap )
                for( int t = 0; t<i; t++ ) {
                    
                    double xB = iterXPoints[t];
                    double yB = iterYPoints[t];
                    
                    int xT = lrint( ( xB - offsetX ) * zoomFactor + w / 2 );
                    int yT = lrint( ( yB + offsetY ) * zoomFactor + h / 2 );   
                    
                    if( xT >= 0 && xT < w &&
                        yT >= 0 && yT < h ) {
                        
                        traceCount[yT][xT] ++;
                        if( traceCount[yT][xT] > maxTraceCount ) {
                            maxTraceCount = traceCount[yT][xT];
                                }
                        }
                    }
                }
            else {
                // exploded
                im.setColor( y * w + x, inColor );
                
                // ignore trace points (center areas are so bright that they
                // overwhelm)
                
                }
            }
        }
    
    
    

    TGAImageConverter conv;
    

    File f( NULL, "test.tga" );
    
    FileOutputStream out( &f );
    
    conv.formatImage( &im, &out );


    if( renderIterMap ) {
        
        Image traceIm( w, h, 3 );
    
        for( int y=0; y<h; y++ ) {
            for( int x=0; x<w; x++ ) {
                double v = log( traceCount[y][x] ) / log( maxTraceCount );
                
                Color c( v, v, v );
                
                traceIm.setColor( y * w + x, c );
                }
            }
        
        File traceF( NULL, "testTrace.tga" );
        
        FileOutputStream traceOut( &traceF );
        
        conv.formatImage( &traceIm, &traceOut );
        }


    delete [] outColors;


    for( int y=0; y<h; y++ ) {
        delete [] traceCount[y];
        }
    delete [] traceCount;

    delete [] iterXPoints;
    delete [] iterYPoints;
    }
