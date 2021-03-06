//***********************************************************************//
//																		 //
//		- "Talk to me like a 3 year old!" Programming Lessons -			 //
//                                                                       //
//		$Author:		DigiBen			DigiBen@GameTutorials.com		 //
//																		 //
//		$Program:		HeightMap6										 //
//																		 //
//		$Description:	Using shaders we show realistic-refracted water  //
//																		 //
//***********************************************************************//

#include "main.h"

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
//
// In this file we just added a third texture coordinate on our terrain for 
// doing our caustic effects.  This is done in SetTextureCoord().  When we
// want to render caustics we just turn on the third texture unit, otherwise
// it won't draw them.
// 

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


// This stores the desired depth that we want to fog
extern float g_FogDepth;

// This is our fog extension function pointer to set a vertice's depth
extern PFNGLFOGCOORDFEXTPROC glFogCoordfEXT;

// This tells us if we want detail texturing turned on or off
extern bool g_bDetail;

// This holds the current tile scale value for our detail texture
extern int g_DetailScale;


///////////////////////////////// HEIGHT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This returns the height into the height map
/////
///////////////////////////////// HEIGHT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

int Height(BYTE *pHeightMap, int X, int Y)
{
	// Make sure we don't go past our array size
	int x = X % MAP_SIZE;					// Error check our x value
	int y = Y % MAP_SIZE;					// Error check our y value

	if(!pHeightMap) return 0;				// Make sure our data is valid

	// Use the equation: index = (x + (y * arrayWidth) ) to find the current height
	return pHeightMap[x + (y * MAP_SIZE)];	// Index into our height array and return the height
}


///////////////////////////////// SET TEXTURE COORD \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This sets the current texture coordinate of the terrain, based on the X and Z
/////
///////////////////////////////// SET TEXTURE COORD \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void SetTextureCoord(float x, float z, float height)
{
	// Find the (u, v) coordinate for the current vertex
	float u =  (float)x / (float)MAP_SIZE;
	float v = -(float)z / (float)MAP_SIZE;
	
	// Give OpenGL the current terrain texture coordinate for our height map
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, u, v);

	// Give OpenGL the current detail texture coordinate for our height map
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, u, v);


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// Give OpenGL the current caustic texture coordinate for our height map
	glMultiTexCoord2fARB(GL_TEXTURE2_ARB, u, v);

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


}


///////////////////////////////// SET FOG COORD \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This sets the volumetric fog for the current vertex with the desired depth
/////
///////////////////////////////// SET FOG COORD \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void SetFogCoord(float depth, float height)
{
	float fogY = 0;

	// Check if the height of this vertex is greater than the depth (needs no fog)
	if(height > depth)
		fogY = 0;
	// Otherwise, calculate the fog depth for the current vertex
	else
		fogY = -(height - depth);

	// Assign the fog coordinate for this vertex using our extension function pointer
	glFogCoordfEXT(fogY);
}


///////////////////////////////// RENDER HEIGHT MAP \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This renders the height map as QUADS
/////
///////////////////////////////// RENDER HEIGHT MAP \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void RenderHeightMap(BYTE pHeightMap[])
{
	int X = 0, Y = 0;						// Create some variables to walk the array with.
	int x, y, z;							// Create some variables for readability
	bool bSwitchSides = false;

	// Make sure our height data is valid
	if(!pHeightMap) return;		
	
	// Activate the first texture ID and bind the tree background to it
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_Texture[TERRAIN_ID]);

	// If we want detail texturing on, let's render the second texture
	if(g_bDetail)
	{
		// Activate the second texture ID and bind the fog texture to it
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glEnable(GL_TEXTURE_2D);
		
		// Here we turn on the COMBINE properties and increase our RGB
		// gamma for the detail texture.  2 seems to work just right.
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 2);
		
		// Bind the detail texture
		glBindTexture(GL_TEXTURE_2D, g_Texture[DETAIL_ID]);
	
		// Now we want to enter the texture matrix.  This will allow us
		// to change the tiling of the detail texture.
		glMatrixMode(GL_TEXTURE);

			// Reset the current matrix and apply our chosen scale value
			glLoadIdentity();
			glScalef((float)g_DetailScale, (float)g_DetailScale, 1);

		// Leave the texture matrix and set us back in the model view matrix
		glMatrixMode(GL_MODELVIEW);
	}

	// We want to render triangle strips
	glBegin( GL_TRIANGLE_STRIP );			

	// Go through all of the rows of the height map
	for ( X = 0; X <= MAP_SIZE; X += STEP_SIZE )
	{
		// Check if we need to render the opposite way for this column
		if(bSwitchSides)
		{	
			// Render a column of the terrain, for this current X.
			// We start at MAP_SIZE and render down to 0.
			for ( Y = MAP_SIZE; Y >= 0; Y -= STEP_SIZE )
			{
				// Get the (X, Y, Z) value for the bottom left vertex		
				x = X;							
				y = Height(pHeightMap, X, Y );	
				z = Y;

				// Set the fog coordinate for this vertex
				SetFogCoord(g_FogDepth, (float)y);

				// Set the current texture coordinate and render the vertex
				SetTextureCoord( (float)x, (float)z, (float)y );
				glVertex3i(x, y, z);		

				// Get the (X, Y, Z) value for the bottom right vertex		
				x = X + STEP_SIZE; 
				y = Height(pHeightMap, X + STEP_SIZE, Y ); 
				z = Y;

				// Set the fog coordinate for this vertex
				SetFogCoord(g_FogDepth, (float)y);

				// Set the current texture coordinate and render the vertex
				SetTextureCoord( (float)x, (float)z,  (float)y);
				glVertex3i(x, y, z);			
			}
		}
		else
		{	
			// Render a column of the terrain, for this current X.
			// We start at 0 and render down up to MAP_SIZE.
			for ( Y = 0; Y <= MAP_SIZE; Y += STEP_SIZE )
			{
				// Get the (X, Y, Z) value for the bottom right vertex		
				x = X + STEP_SIZE; 
				y = Height(pHeightMap, X + STEP_SIZE, Y ); 
				z = Y;

				// Set the fog coordinate for this vertex
				SetFogCoord(g_FogDepth, (float)y);

				// Set the current texture coordinate and render the vertex
				SetTextureCoord( (float)x, (float)z,  (float)y);
				glVertex3i(x, y, z);

				// Get the (X, Y, Z) value for the bottom left vertex		
				x = X;							
				y = Height(pHeightMap, X, Y );	
				z = Y;							

				// Set the fog coordinate for this vertex
				SetFogCoord(g_FogDepth, (float)y);

				// Set the current texture coordinate and render the vertex
				SetTextureCoord( (float)x, (float)z, (float)y );
				glVertex3i(x, y, z);		
			}
		}

		// Switch the direction the column renders to allow the fluid tri strips
		bSwitchSides = !bSwitchSides;
	}

	// Stop rendering triangle strips
	glEnd();

	// Turn the second multitexture pass off
	glActiveTextureARB(GL_TEXTURE1_ARB);
    glDisable(GL_TEXTURE_2D);

	// Turn the first multitexture pass off
	glActiveTextureARB(GL_TEXTURE0_ARB);		
    glDisable(GL_TEXTURE_2D);
}


///////////////////////////////// LOAD RAW FILE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This loads a .raw file into an array of bytes.  Each value is a height value.
/////
///////////////////////////////// LOAD RAW FILE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void LoadRawFile(LPSTR strName, int nSize, BYTE *pHeightMap)
{
	FILE *pFile = NULL;

	// Let's open the file in Read/Binary mode.
	pFile = fopen( strName, "rb" );

	// Check to see if we found the file and could open it
	if ( pFile == NULL )	
	{
		// Display our error message and stop the function
		MessageBox(NULL, "Can't find the height map!", "Error", MB_OK);
		return;
	}

	// Here we load the .raw file into our pHeightMap data array.
	// We are only reading in '1', and the size is the (width * height)
	fread( pHeightMap, 1, nSize, pFile );

	// After we read the data, it's a good idea to check if everything read fine.
	int result = ferror( pFile );

	// Check if we received an error.
	if (result)
	{
		MessageBox(NULL, "Can't get data!", "Error", MB_OK);
	}

	// Close the file.
	fclose(pFile);
}


/////////////////////////////////////////////////////////////////////////////////
//
// * QUICK NOTES * 
//
// All we added was a third texture coordinate for our terrain for rendering
// caustic effects for underwater.
//
//
// �2000-2005 GameTutorials   
