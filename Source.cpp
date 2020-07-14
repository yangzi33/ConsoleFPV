#include <iostream>
#include <chrono>
using namespace std;

#include <Windows.h>

int nScreenWidth = 120; // Width of the console
int nScreenHeight = 40; // Height of the console

float fPlayerX = 8.0f;  // Player's x coor
float fPlayerY = 8.0f;  // Player's y coor
float fPlayerA = 0.0f;  // Angle of the player's pov

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.1415926 / 4;
float fDepth = 16.0f;

int main()
{
    // Creates screen buffer
    wchar_t *screen = new wchar_t[nScreenWidth * nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0,
            NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    wstring map;

    map += L"################";
    map += L"#              #";
    map += L"#              #";
    map += L"#              #";
    map += L"######         #";
    map += L"#              #";
    map += L"#              #";
    map += L"#        #######";
    map += L"#              #";
    map += L"#              #";
    map += L"#              #";
    map += L"#              #";
    map += L"#              #";
    map += L"#              #";
    map += L"#              #";
    map += L"################";

    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    // GAME LOOP
    while (true)
    {
        tp2 = chrono::system_clock::now();  // Get Sys time
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapseTime.count();

        // Control that handle CW rotations
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
            fPlayerA -= (0.5f) * fElapsedTime;

        // Control that handle CCW rotations
        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
            fPlayerA += (0.5f) * fElapsedTime;

        // Control that handle forward movements
        if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
        {
            fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
            {
                fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }

        // Control that handle backward movements
        if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {
            fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;

            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
            {
                fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }

        for (int x = 0; x < nScreenWidth; x++)
        {
            // Calculate the projectted ray angle to R3 for each column
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

            float fDistanceToWall = 0;
            bool bHitWall = false; // Indicates whether the player hits the wall
            bool bBoundary = false;

            float fEyeX = sinf(fRayAngle); // Unit vector for ray in R3
            float fEyeY = cosf(fRayAngle);

            while (!bHitWall && fDistanceToWall < fDepth)
            {

                fDistanceToWall += 0.1f;

                int nTestX = (int)(fPlayerX + fEyeX + fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY + fDistanceToWall);

                // Check if the ray is out of bounds
                if (nTestX < 0 ||
                    nTestX >= nMapWidth ||
                    nTestY < 0 ||
                    nTestY >= nMapHeight)
                {
                    bHitWall = true;
                    // Just set the distance to maximum depth
                    fDistanceToWall = fDepth;
                }
                else
                {
                    // Ray in range
                    // Test whether a wall block in ray
                    if (map[nMapWidth * nTestY + nTestX] == '#')
                    {
                       bHitWall = true;

                       vector<pair<float, float>> p; // distance, dot

                       for (int tx = 0; tx < 2; tx++)
                       {
                           for (int ty = 0; ty < 2; ty++)
                           {
                               float vy = (float)nTestY + ty - fPlayerY;
                               float vx = (float)nTestX + tx - fPlayerX;
                               float d = sqrt(vx * vx + vy * vy);
                               float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                               p.push_back(make_pair(d, dot));
                           }

                           // Sort the vector pairs in ascending order in distance
                           sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right)
                                   {return left.first < right.first;});

                           float fBound = 0.01;
                           if (acos(p.at(0).second) < fBound)
                               bBoundary = true;
                           if (acos(p.at(1).second) < fBound)
                               bBoundary = true;
                           if (acos(p.at(2).second) < fBound)
                               bBoundary = true;

                       }
                    }
                }
            }

            // Distance to floor and ceiling
            int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
            int nFloor = nSreenHeight- nCeiling;

            // ASCII shade variables
            // Refer to extended ASCII codes in www.asciitable.com
            short nShade = ' ';

            if (fDistanceToWall <= fDepth / 4.0f)       // Very close
                nShade = 0x2588;
            else if (fDistanceToWall < fDepth / 3.0f)   // Close
                nShade = 0x2593;
            else if (fDistanceToWall < fDepth / 2.0f)   // Far
                nShade = 0x2592;
            else if (fDistanceToWall < fDepth)          // Very far
                nShade = 0x2591;
            else                                        // Too far to compute
                nShade = ' ';

            if (bBoundary)
                nShade = ' ';                           // Black out the boundary

            for (int y = 0; y < nScreenHeight; y++)
            {
                if (y < nCeiling)
                    screen[y * nScreenWidth + x] = ' ';  // Ceiling
                else if (y > nCeiling && y <= nFloor)
                    screen[y * nScreenWidth + x] = '#';  // Wall
                else
                {
                    // Shade floor with respect to distance
                    float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2));
                    if  (b < 0.25)      nShade = '#';
                    else if (b < 0.5)   nShade = 'x';
                    else if (b < 0.75)  nShade = '.';
                    else if (b < 0.9)   nShade = '-';
                    else                nShade = ' ';
                    screen[y * nScreenWidth + x] = nShade;
                }
            }


        }

        // Stats display
        swprintf_s(screenm 40, L"X-%3.2f, Y=%3.2f, A=%3.2f, FPS=%3.2f ",
                fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

        // Mini map display
        for (int nx = 0; nx < nMapWidth; nx++)
        {
            for (int ny = 0; ny < nMapHeight; ny ++)
            {
                screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
            }
        }

        // Use 'P' to indicate player's position
        screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 'P';

        screen[nScreenWidth * nScreenHeight - 1] = '\0';
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight,
                { 0,0 }, &dwBytesWritten);
    }
    return 0;


}
