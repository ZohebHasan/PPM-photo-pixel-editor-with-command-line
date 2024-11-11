#include "hw2.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h> 

#define PPM 100
#define SBU 200
#define PIXEL_LENGTH 3


char *inputFilePath = NULL;
char *outputFilePath = NULL;
bool containsI = false, containsO = false, containsC = false, containsP = false, containsR = false; 
int elementsOfC[4] = {0};
int elementsOfP[2] = {0};
char *elementsOfRstr[3] = {NULL};
int elementsOfRint[2] = {0};
int backupWidth, backupHeight;
int* originalPixels = NULL;        
int copiedPixelsLen = 0;
int colorTableLen = 0;
int **colorTable;
int originalPixelsLen = 0;
int effectiveWidthRegion, effectiveHeightRegion;
int* copiedPixels; 

bool containsRarg(int argumentsLength, char **argumentsArray);
int checkArgs(int argumentsLength, char **argumentsArray);
int checkFileType(char *filePath);
bool splitArgument(char *argument, char *option);
bool validFile(char *filePath, char option);
FILE *getFile(char *filePath, char task );
void pastePixels(FILE *outputFile, int inputFileType, int outputFileType,  int startingRow, int startingCol,  int copiedFileWidth, int copiedFileHeight);
void copyPixels(FILE *file, int fileType, int startingRow, int startingCol, int copiedWidthRegion, int copiedHeightRegion);
void clonePixels(FILE *file, int fileType);
void loadAndSave( FILE *inpFile , FILE *outpFile , int inputFileType, int outputFileType);
void saveFile(FILE * outputFile, int inputFileType, int outputFileType);


bool splitArgument(char *argument, char *option){
    if (argument == NULL) {
        return false;
    }
    else{
      int index = 0;
      char *token;
      token = strtok(argument, ","); 

      if(strcmp(option, "r") == 0){
        int count = 0;
        while (token != NULL) {
            if (count < 3) {
                elementsOfRstr[count] = token; 
            } 
            else if (count < 5) {
                elementsOfRint[index] = atoi(token);
                index++;
            }
            token = strtok(NULL, ",");
            count++;
        }
        return count == 5;
      }
      else if(strcmp(option, "p") == 0){ 
          while (token != NULL) {
              if (index < 2) {
                  elementsOfP[index] = atoi(token); //changed
              }
              token = strtok(NULL, ",");
              index++;
          }
          return index == 2;
      }
      else{
          while (token != NULL) {
              if (index < 4) {
                  elementsOfC[index] = atoi(token); 
              }
              token = strtok(NULL, ",");
              index++;
          }
              if (index != 4) {
                  return false;
              }
        return index == 4;
      }
    }
    return true;
}

int checkFileType(char *filePath) {
    char *fileExtension = strrchr(filePath, '.');

    if ((fileExtension == NULL) || (fileExtension == filePath)) {
        return -1; 
    } 
    else if (strcmp(fileExtension, ".ppm") == 0) {
        return PPM;
    } 
    else if (strcmp(fileExtension, ".sbu") == 0) {
        return SBU;
    } 
    return -1; 
}

bool validFile(char *filePath, char option){
  FILE *file;
  if(filePath == NULL){
    return false;
  }
  char mode[2] = {option, '\0'};
  file = fopen(filePath, mode);

  if(file == NULL){
    return false;
  }
  else if(checkFileType(filePath) == -1){
    return false;
  }

  fclose(file);
  if(option == 'w') { //experiment;
    remove(filePath); 
  }
  
  return true;
}

FILE *getFile(char *filePath, char task ) { //gimmick
    FILE *file = NULL;
    if(validFile(filePath, task) == true){ 
         file = fopen(filePath, task == 'r' ? "r" : "w");
    }
    return file;
}

bool containsRarg(int argumentsLength, char **argumentsArray) {
    for (int i = 1; i < argumentsLength; i++) { 
        if (strcmp(argumentsArray[i], "-r") == 0) {
            if ((i + 1 >= argumentsLength) || (argumentsArray[i + 1][0] == '-')) {
                return false;
            }
        }
    }
    return true; 
}

int checkArgs(int argumentsLength, char **argumentsArray) {
    int option;
    int argCount = 0;
    int unrecongnizedArg = false;
    int iCt = 0, oCt = 0; 
    bool validInputFilePath = false, validInputFile = false,  validOutputFilePath = false, validCargument = false, validPargument = false, containsRargument = false, validRargument = false;

    while ((option = getopt(argumentsLength, argumentsArray, "i:o:c:p:r")) != -1) {
        switch (option) {
            case 'i':
                inputFilePath = optarg;
                if (validFile(inputFilePath, 'r') == true) {
                    validInputFile = true;   
                }
                if(checkFileType(inputFilePath) != -1){
                    validInputFilePath = true;
                }
                containsI = true;
                argCount++;
                iCt++;
                break;
            case 'o':
                outputFilePath = optarg;
                if (validFile(outputFilePath, 'w') == true) {
                    validOutputFilePath = true;      
                }
                containsO = true;
                argCount++;
                oCt++;
                break;
            case 'c':        
                if(splitArgument(optarg, "c") == true){
                    validCargument = true;
                }
                containsC = true;
                argCount++;
                break;
            case 'p':
                containsP = true;
                validPargument = splitArgument(optarg, "p");
                argCount++;
                break;
            case 'r':     
                containsR = true; 
                containsRargument = containsRarg(argumentsLength, argumentsArray);
                if(optarg == NULL){
                    validRargument = false;
                    argCount++;
                    break;
                }  
                if(splitArgument(optarg, "r") == true){
                    validRargument = true;
                }
                argCount++;
                break;
            default:
                unrecongnizedArg = true;
                break;
        }
    }

    if (inputFilePath == NULL || outputFilePath == NULL || argCount < 2) {
        return MISSING_ARGUMENT;
    } 
    else if (iCt < 1 || oCt < 1) {
        return MISSING_ARGUMENT;
    }
    else if(containsI && !validInputFilePath){
        return MISSING_ARGUMENT;
    }
    else if(containsR && !containsRargument){
        return MISSING_ARGUMENT;
    }
    else if (unrecongnizedArg) {
        return UNRECOGNIZED_ARGUMENT;
    } 
    else if (iCt > 1 || oCt > 1) {
        return DUPLICATE_ARGUMENT;
    } 
    else if (containsI && !validInputFile) {
        return INPUT_FILE_MISSING;
    } 
    else if (containsO && !validOutputFilePath) {
        return OUTPUT_FILE_UNWRITABLE;
    } 
    else if (containsP && !containsC) {
        return C_ARGUMENT_MISSING;
    } 
    else if (containsC && !validCargument) {
        return C_ARGUMENT_INVALID;
    } 
    else if (containsP && !validPargument) {
        return P_ARGUMENT_INVALID;
    } 
    else if(containsR && !validRargument){
        return R_ARGUMENT_INVALID;
    }
    else {
        return 0;
    }
}

void clonePixels(FILE *file, int fileType){
    int width, height;
    int r, g, b;
 
    if (fileType == PPM) {
        fscanf(file, "%*s %d %d %*d", &width, &height);
  
        backupWidth = width;
        backupHeight = height;
        originalPixels = (int*)malloc(width * height * PIXEL_LENGTH * sizeof(int));
   

        for (int row = 0; row < height; row++) {
            for (int col = 0; col < width; col++) {
                fscanf(file, "%d %d %d", &r, &g, &b); 
                int index = (row * width + col) * PIXEL_LENGTH;
                originalPixels[index] = r;
                originalPixels[index + 1] = g;
                originalPixels[index + 2] = b;
                originalPixelsLen += 3;
            }
            
        }      
    } 
    else if (fileType == SBU) {
        fscanf(file, "%*s %d %d %d", &width, &height, &colorTableLen);
        backupWidth = width;
        backupHeight = height;
        colorTable = malloc(colorTableLen * sizeof(int *));
        for(int i = 0; i < colorTableLen; i++){
            colorTable[i] = malloc(PIXEL_LENGTH * sizeof(int));
        }
        originalPixels = (int*)malloc(width * height * PIXEL_LENGTH * sizeof(int));
      

        for (int i = 0; i < colorTableLen; i++) {
            fscanf(file, "%d %d %d", &r, &g, &b);
            colorTable[i][0] = r;
            colorTable[i][1] = g;
            colorTable[i][2] = b;
        }
        
        char buffer[20];
        
        while (fscanf(file, "%s", buffer) == 1 && originalPixelsLen < width * height) {
            if (buffer[0] == '*') {
                int repeats, currentColorIndex;
                sscanf(buffer, "*%d", &repeats);
                fscanf(file, "%d", &currentColorIndex);
                for (int i = 0; i < repeats; i++) {
                    int index = originalPixelsLen * PIXEL_LENGTH;     
                    originalPixels[index] = colorTable[currentColorIndex][0];
                    originalPixels[index + 1] = colorTable[currentColorIndex][1];
                    originalPixels[index + 2] = colorTable[currentColorIndex][2];
                    originalPixelsLen++;
                }
            } 
            else {
                int currentColorIndex = atoi(buffer);
                int index = originalPixelsLen * PIXEL_LENGTH;
                originalPixels[index] = colorTable[currentColorIndex][0];
                originalPixels[index + 1] = colorTable[currentColorIndex][1];
                originalPixels[index + 2] = colorTable[currentColorIndex][2];
                originalPixelsLen++;
            }
        }
        originalPixelsLen *= PIXEL_LENGTH;
    }
    rewind(file);
}

void pastePixels(FILE *outputFile, int inputFileType, int outputFileType,  int startingRow, int startingCol,  int copiedFileWidth, int copiedFileHeight){
    int r, g, b;
    if(outputFileType == PPM){ 
        fprintf(outputFile, "P3\n%d %d\n255\n", copiedFileWidth, copiedFileHeight);
        for (int row = 0; row < copiedFileHeight; row++) { 
            for (int col = 0; col < copiedFileWidth; col++) {
                if (row >= startingRow && row < startingRow + effectiveHeightRegion && col >= startingCol && col < startingCol + effectiveWidthRegion) {
                    int copiedIndex = ((row - startingRow) * effectiveWidthRegion + (col - startingCol)) * PIXEL_LENGTH;
                    if (copiedIndex < copiedPixelsLen) {
                        fprintf(outputFile, "%d %d %d ", copiedPixels[copiedIndex], copiedPixels[copiedIndex + 1], copiedPixels[copiedIndex + 2]);
                    }
                } 
                else {
                    int index = (row * copiedFileWidth + col) * PIXEL_LENGTH;
                    fprintf(outputFile, "%d %d %d ", originalPixels[index], originalPixels[index + 1], originalPixels[index + 2]);
                }
            }
            fprintf(outputFile, "\n");
        }
    }
    else if(outputFileType == SBU){
        
        for (int row = 0; row < copiedFileHeight; row++) {
            for (int col = 0; col < copiedFileWidth; col++) {
                int index = (row * copiedFileWidth + col) * PIXEL_LENGTH;
                if (row >= startingRow && row < startingRow + effectiveHeightRegion && col >= startingCol && col < startingCol + effectiveWidthRegion) {
                    int copiedIndex = ((row - startingRow) * effectiveWidthRegion + (col - startingCol)) * PIXEL_LENGTH;
                    if (copiedIndex + 2 < copiedPixelsLen) { 
                        originalPixels[index] = copiedPixels[copiedIndex];
                        originalPixels[index+1] = copiedPixels[copiedIndex+1];
                        originalPixels[index+2] = copiedPixels[copiedIndex+2];
                    }
                }
            }
        }

        if (inputFileType == PPM) {
            colorTable = NULL;
            colorTableLen = 0;
            
            for (int i = 0; i < originalPixelsLen; i += PIXEL_LENGTH) {
                bool found = false;
                for (int j = 0; j < colorTableLen; j++) {
                    if (colorTable[j][0] == originalPixels[i] && colorTable[j][1] == originalPixels[i+1] && colorTable[j][2] == originalPixels[i+2]) {
                        found = true;
                        break;
                    }
                }
                if (found == false) {
                    colorTable = realloc(colorTable, (colorTableLen + 1) * sizeof(int *));
                    colorTable[colorTableLen] = malloc(PIXEL_LENGTH * sizeof(int));
                    colorTable[colorTableLen][0] = originalPixels[i];
                    colorTable[colorTableLen][1] = originalPixels[i+1];
                    colorTable[colorTableLen][2] = originalPixels[i+2];
                    colorTableLen++;
                }
            }
        }
        
        fprintf(outputFile, "SBU\n%d %d\n%d\n", copiedFileWidth, copiedFileHeight, colorTableLen);
        for (int i = 0; i < colorTableLen; i++) {
            fprintf(outputFile, "%d %d %d ", colorTable[i][0], colorTable[i][1], colorTable[i][2]);
        }
        fprintf(outputFile,"\n");

        
        int lastColorIndex = -1, repeatCount = 0;
        for (int i = 0; i < originalPixelsLen; i += PIXEL_LENGTH) {
            r = originalPixels[i];
            g = originalPixels[i + 1];
            b = originalPixels[i + 2];

            int currentColorIndex = -1;
            for (int j = 0; j < colorTableLen; j++) {
                if (colorTable[j][0] == r && colorTable[j][1] == g && colorTable[j][2] == b) {
                    currentColorIndex = j;
                    break;
                }
            }

            if (currentColorIndex == -1) {
                fprintf(stderr, "Error: Color not found in color table.\n");
                continue;
            }

            if (currentColorIndex == lastColorIndex) {
                repeatCount++;
            } 
            else {
                if (lastColorIndex != -1) {
                    if (repeatCount > 1){
                        fprintf(outputFile, "*%d %d ", repeatCount, lastColorIndex);
                    } 
                    else{
                        fprintf(outputFile, "%d ", lastColorIndex);
                    } 
                }
                lastColorIndex = currentColorIndex;
                repeatCount = 1;
            }
        }

        if (lastColorIndex != -1) {
            if (repeatCount > 1){
                fprintf(outputFile, "*%d %d", repeatCount, lastColorIndex);
            } 
            else{
                 fprintf(outputFile, "%d", lastColorIndex);
            }
        }
    }
    rewind(outputFile);
}

void copyPixels(FILE *file, int fileType, int startingRow, int startingCol, int copiedWidthRegion, int copiedHeightRegion) {
    int width, height, r, g, b;
   
    int index = 0;
    if (fileType == PPM) {  
        index = 0;
        fscanf(file, "%*s %d %d %*d", &width, &height);
        effectiveWidthRegion = (startingCol + copiedWidthRegion > width) ? width - startingCol : copiedWidthRegion;
        effectiveHeightRegion = (startingRow + copiedHeightRegion > height) ? height - startingRow : copiedHeightRegion;
        copiedPixelsLen = effectiveWidthRegion * effectiveHeightRegion * PIXEL_LENGTH;
        copiedPixels = (int*)malloc(copiedPixelsLen * sizeof(int));
        
        if (copiedPixels == NULL) {
            return;
        }
        for (int row = 0; row < height; row++) {
            for (int col = 0; col < width; col++) {
                fscanf(file, "%d %d %d", &r, &g, &b);           
                if (row >= startingRow && row < startingRow + effectiveHeightRegion && col >= startingCol && col < startingCol + effectiveWidthRegion) { 
                    copiedPixels[index] = r;
                    copiedPixels[index + 1] = g;
                    copiedPixels[index + 2] = b;
                    index += 3;
                } 
            }
        }
    } 
       
    
    else if (fileType == SBU) {
        int clrTableLen = 0;
        fscanf(file, "%*s %d %d %d", &width, &height, &clrTableLen);
        int tempPixels[width * height * PIXEL_LENGTH * sizeof(int)];
        
        int **clrTable = malloc(clrTableLen * sizeof(int *));

        for(int i = 0; i < clrTableLen; i++){
            clrTable[i] = malloc(PIXEL_LENGTH * sizeof(int));
        }
        
        for (int i = 0; i < clrTableLen; i++) {
            fscanf(file, "%d %d %d", &r, &g, &b);
            clrTable[i][0] = r;
            clrTable[i][1] = g;
            clrTable[i][2] = b;
        }

        char buffer[20];
        int pixelsArrLen = 0;
        while (fscanf(file, "%s", buffer) == 1 && pixelsArrLen < width * height) {
            if (buffer[0] == '*') {
                int repeats, currentColorIndex;
                sscanf(buffer, "*%d", &repeats);
                fscanf(file, "%d", &currentColorIndex);
                for (int i = 0; i < repeats; i++) {
                    index = pixelsArrLen * PIXEL_LENGTH;
                    tempPixels[index] = clrTable[currentColorIndex][0];
                    tempPixels[index + 1] = clrTable[currentColorIndex][1];
                    tempPixels[index + 2] = clrTable[currentColorIndex][2];
                    
                    pixelsArrLen++;
                }
            } else {
                int currentColorIndex = atoi(buffer);
                index = pixelsArrLen * PIXEL_LENGTH;
                tempPixels[index] = clrTable[currentColorIndex][0];
                tempPixels[index + 1] = clrTable[currentColorIndex][1];
                tempPixels[index + 2] = clrTable[currentColorIndex][2];

                pixelsArrLen++;
            }
        }
        
 
        index = 0; 
        int pixelIndex; 

        effectiveWidthRegion = (startingCol + copiedWidthRegion > width) ? width - startingCol : copiedWidthRegion;
        effectiveHeightRegion = (startingRow + copiedHeightRegion > height) ? height - startingRow : copiedHeightRegion;
        copiedPixelsLen = effectiveWidthRegion * effectiveHeightRegion * PIXEL_LENGTH;
        copiedPixels = (int*)malloc(copiedPixelsLen * sizeof(int));
        for (int row = startingRow; row < startingRow + effectiveHeightRegion; row++) {
            for (int col = startingCol; col < startingCol + effectiveWidthRegion; col++) {
                pixelIndex = (row * width + col) * PIXEL_LENGTH;
                copiedPixels[index] = tempPixels[pixelIndex];
                copiedPixels[index + 1] = tempPixels[pixelIndex + 1];
                copiedPixels[index + 2] = tempPixels[pixelIndex + 2];
                index += 3; 
            }
        } 

    for(int i = 0; i < clrTableLen; i++){
        free(clrTable[i]);
    }
    free(clrTable);

    }

    rewind(file); 
}

void loadAndSave( FILE* inpFile , FILE *outpFile , int inputFileType, int outputFileType){
    clonePixels(inpFile, inputFileType);
    saveFile(outpFile, inputFileType, outputFileType);

}

void saveFile(FILE * outputFile, int inputFileType, int outputFileType ){
    int r, g, b;
    if(outputFileType == PPM){ 
        fprintf(outputFile, "P3\n%d %d\n255\n", backupWidth, backupHeight);
        for (int row = 0; row < backupHeight; row++) { 
            for (int col = 0; col < backupWidth; col++) {
                int index = (row * backupWidth + col) * PIXEL_LENGTH;
                fprintf(outputFile, "%d %d %d ", originalPixels[index], originalPixels[index + 1], originalPixels[index + 2]);

            }
            fprintf(outputFile, "\n");
        }
    }
    else if(outputFileType == SBU){       

        if (inputFileType == PPM) {
            colorTable = NULL;
            colorTableLen = 0;
            
            for (int i = 0; i < originalPixelsLen; i += PIXEL_LENGTH) {
                bool found = false;
                for (int j = 0; j < colorTableLen; j++) {
                    if (colorTable[j][0] == originalPixels[i] && colorTable[j][1] == originalPixels[i+1] && colorTable[j][2] == originalPixels[i+2]) {
                        found = true;
                        break;
                    }
                }
                if (found == false) {
                    colorTable = realloc(colorTable, (colorTableLen + 1) * sizeof(int *));
                    colorTable[colorTableLen] = malloc(PIXEL_LENGTH * sizeof(int));
                    colorTable[colorTableLen][0] = originalPixels[i];
                    colorTable[colorTableLen][1] = originalPixels[i+1];
                    colorTable[colorTableLen][2] = originalPixels[i+2];
                    colorTableLen++;
                }
            }
        }
        
        fprintf(outputFile, "SBU\n%d %d\n%d\n", backupWidth, backupHeight, colorTableLen);
        for (int i = 0; i < colorTableLen; i++) {
            fprintf(outputFile, "%d %d %d ", colorTable[i][0], colorTable[i][1], colorTable[i][2]);
        }
        fprintf(outputFile,"\n");

        
        int lastColorIndex = -1, repeatCount = 0;
        for (int i = 0; i < originalPixelsLen; i += PIXEL_LENGTH) {
            r = originalPixels[i];
            g = originalPixels[i + 1];
            b = originalPixels[i + 2];

            int currentColorIndex = -1;
            for (int j = 0; j < colorTableLen; j++) {
                if (colorTable[j][0] == r && colorTable[j][1] == g && colorTable[j][2] == b) {
                    currentColorIndex = j;
                    break;
                }
            }

            if (currentColorIndex == -1) {
                fprintf(stderr, "Error: Color not found in color table.\n");
                continue;
            }

            if (currentColorIndex == lastColorIndex) {
                repeatCount++;
            } 
            else {
                if (lastColorIndex != -1) {
                    if (repeatCount > 1){
                        fprintf(outputFile, "*%d %d ", repeatCount, lastColorIndex);
                    } 
                    else{
                        fprintf(outputFile, "%d ", lastColorIndex);
                    } 
                }
                lastColorIndex = currentColorIndex;
                repeatCount = 1;
            }
        }

        if (lastColorIndex != -1) {
            if (repeatCount > 1){
                fprintf(outputFile, "*%d %d", repeatCount, lastColorIndex);
            } 
            else{
                 fprintf(outputFile, "%d", lastColorIndex);
            }
        }
    }
    rewind(outputFile);
}

int main(int argc, char **argv) {
    int returnValue = checkArgs(argc, argv);
    if (returnValue != 0){
        printf("Invalid input, return value is not 0. Return value is: %d\n" , returnValue);
        return returnValue;
    }
    FILE *inputFile = getFile(inputFilePath, 'r');
    FILE *outputFile = getFile(outputFilePath, 'w');
    loadAndSave(inputFile, outputFile, checkFileType(inputFilePath), checkFileType(outputFilePath));
    fclose(inputFile);
    fclose(outputFile);

    // clonePixels(inputFile, checkFileType(inputFilePath));


    if(containsC == true){ 
        FILE *inpFile = getFile(inputFilePath, 'r');
        copyPixels(inpFile, checkFileType(inputFilePath), elementsOfC[0], elementsOfC[1] , elementsOfC[2], elementsOfC[3]);
        fclose(inpFile);
    }
    if(containsP == true){ 
        FILE *outpFile = getFile(outputFilePath, 'w');
        pastePixels(outpFile, checkFileType(inputFilePath), checkFileType(outputFilePath), elementsOfP[0], elementsOfP[1], backupWidth, backupHeight);  
        fclose(outpFile);
    }
    if(containsR == true){ //print
        //logic
    }

    free(originalPixels);
    for(int i = 0; i < colorTableLen; i++){
        free(colorTable[i]);
    }
    free(colorTable);
    free(copiedPixels);
 
    return 0;
}
