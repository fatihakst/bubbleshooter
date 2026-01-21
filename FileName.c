/*
BUBBLE SHOOTER GAME
*/


#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define LINES 9
#define COL 20
#define MAX_LINES 20
#define NUM_COLORS 6
#define MAX_HEALTH 5

int bubbleShooterTextSize = 70;
int PlayTextSize = 50;
int howToPlayTextSize = 50;
int exitTextSize = 50;
int rectangleWidth = 80;
int rectangleHeight = 370;

Sound ExplosionSound;
Sound ShotSound;
Texture2D arkaPlan;
Texture2D howToPlayTexture;

typedef enum GameState {
	GAME,
	MENU,
	HOW_TO_PLAY,
	FINISH
}GameState;

GameState gameState;

typedef struct {

	int radius;
	Color color;
	bool active;
	Vector2 position;
	Vector2 speed;
	Texture2D texture;

}Ball;

typedef struct {

	int radius;
	Color color;
	bool active;
	bool exploded;
	Vector2 position;
	Texture2D texture;

}Bubble;

// Menü öðeleri arasýnda gezinmek için kullanacaðýmýz deðiþken
int selectedMenuItemIndex = 0;  // Baþlangýçta ilk menü öðesine iþaret eder
int numberOfMenuItems = 3; // Menünün içerdiði toplam öðe sayýsý

Rectangle startButtonBounds;
Rectangle howToPlayButtonBounds;
Rectangle finishButtonBounds;

Ball ball = { 0 };
Ball ball2 = { 0 };

Bubble bubble[MAX_LINES][COL];
Color color = { 0 };
Vector2 bubbleSize = { 0,0 };
int health = MAX_HEALTH;
static bool gameOver = false;
bool isBallHitted = false;

// Addition'dan yeni oluþan bubble'ýn i ve j deðerini döndürme ve explode fonksiyonuna bu deðerleri kullanarak parametre gönderme
int lineCounter;
int columnCounter;

static int point = 0;
static int highPoint = 0;

const int ScreenWidth = 971;
const int ScreenHeight = 725;

void InitGame(void);
void UpdateGame(void);
static void DrawGame(void);
int Addition(int i, int j);
void UnloadGame();

// Menü için eklenenler
void UpdateMenu();
void DrawMenu();

// Baðlý olan ayný renkli bubble larý patlatma fonksiyonlarý
int MarkAndExplodeConnectedBubbles(int i, int j);
int CountConnectedBubbles(int i, int j, int color); // en az 3 tane baðlý bubble olup olmadýðýný kontrol eder
int ExplodeConnectedBubbles(); //eðer en az 3 baðlý bubble varsa hepsini patlatýr (umarým)

// Havada asýlý kalan toplarýn kontrolü (havada top varsa patlat)
void checkExtraExplode();

Color colorPalette[NUM_COLORS] = {
	{ 253, 249, 0, 255 } ,    // Yellow
	 { 0, 121, 241, 255} ,    // Blue
	{ 255, 161, 0, 255 } ,   // Orange
	{ 230, 41, 55, 255 } ,    // Red
	{ 200, 122, 255, 255 }, // Purple
	{ 0, 228, 48, 255 }      // Green
};

int main(void) {

	InitWindow(ScreenWidth, ScreenHeight, "Game");
	InitAudioDevice();

	gameState = MENU;

	InitGame();
	SetTargetFPS(60);


	// Menü ekranýnýn týklanabilir alanlarýný tanýmlama
	Rectangle startButtonBounds = { GetScreenWidth() / 2 - rectangleHeight / 2, 280, rectangleHeight, rectangleWidth };
	Rectangle howToPlayButtonBounds = { GetScreenWidth() / 2 - rectangleHeight / 2, 400, rectangleHeight, rectangleWidth };
	Rectangle finishButtonBounds = { GetScreenWidth() / 2 - rectangleHeight / 2, 520, rectangleHeight, rectangleWidth };


	// Bubble için texturelerin yüklenmesi 
	for (int i = 0; i < MAX_LINES; i++) {
		for (int j = 0; j < COL; j++) {
			bubble[i][j].texture = LoadTexture("..\\bubbleTexture.png");
			continue;
		}
	}

	// Ses dosyalarýnýn yüklenmesi
	ExplosionSound = LoadSound("..\\bubbling2.wav");
	ShotSound = LoadSound("..\\shot.wav");


	// Arka plan resmini yükleme
	arkaPlan = LoadTexture("..\\arkaPlan.png");

	// How to Play için gerekli görseli yükle
	howToPlayTexture = LoadTexture("..\\HowToPlay.png");

	while (!WindowShouldClose()) {
		// Oyun durumuna göre iþlemler
		switch (gameState) {
		case GAME:
		{
			// Oyun güncelleniyor
			UpdateGame();
			DrawGame();
			if (IsKeyPressed(KEY_B)) {
				gameState = MENU;
			}
			break;
		}
		case MENU:
		{
			// Menü güncelleniyor
			UpdateMenu(); // Menü öðelerinin týklanabilir alanlarýný iþlemek için
			DrawMenu();
			break;
		}
		case HOW_TO_PLAY:
		{
			// How to Play ekraný
			if (IsKeyPressed(KEY_B)) {
				gameState = MENU;
			}
			BeginDrawing();
			ClearBackground(RAYWHITE);
			DrawTexture(howToPlayTexture, 0, 0, WHITE);
			EndDrawing();
			break;
		}
		case FINISH:
		{
			gameState = FINISH;
			break;
		}
		}
		// Oyun döngüsünü sonlandýr
		if (gameState == FINISH) {
			break;
		}


	}
	UnloadTexture(howToPlayTexture); // How to Play görselini kaldýr
	UnloadGame();
	CloseWindow();

	return 0;

}
void InitGame() {

	
	// Ball Özellikleri
	ball.radius = 20;
	ball.position = (Vector2){ ScreenWidth / 2, ScreenHeight - 50 };
	ball.speed = (Vector2){ 0, 0 };
	ball.color = colorPalette[GetRandomValue(0, NUM_COLORS - 1)];
	ball.active = false;
	ball.texture = LoadTexture("..\\bubbleTexture.png");

	// Ball'a Rengini Verecek Topun Özellikleri
	ball2.radius = 15;
	ball2.position = (Vector2){ ScreenWidth / 2 - 50, ScreenHeight - 40 };
	ball2.speed = (Vector2){ 0, 0 };
	ball2.color = colorPalette[GetRandomValue(0, NUM_COLORS - 1)];
	ball2.active = true;
	ball2.texture = LoadTexture("..\\bubbleTexture.png");


	bubbleSize = (Vector2){ 40 , 40 };
	health = MAX_HEALTH;
	point = 0;

	// Bubble larýn ilk pozisyonlarý
	for (int i = 0; i < MAX_LINES; i++) {
		for (int j = 0; j < COL; j++) {
			bubble[i][j].exploded = false;
			if (i % 2 == 0) {
				bubble[i][j].position = (Vector2){ j * bubbleSize.x + ball.radius + j * 9 , i * bubbleSize.y + ball.radius };
			}
			else {
				if (j == 19)
					continue;
				bubble[i][j].position = (Vector2){ j * bubbleSize.x + ball.radius + ball.radius + j * 9, i * bubbleSize.y + ball.radius };
			} // Sadece görmek istediðimiz bubble lara aktiflik vermek
			if (i < LINES) {
				bubble[i][j].active = true;
				bubble[i][j].color = colorPalette[GetRandomValue(0, NUM_COLORS - 1)];
			}
			else
				bubble[i][j].active = false;
		}
	}
}
int Addition(int i, int j) {

	// Çarpma açýlarýný kontrol eden fonksiyon
	if (i % 2 == 0) {
		if ((ball.position.x - bubble[i][j].position.x) > 0) {

			// Sað alttaki topun temasý 
			if ((ball.position.y - bubble[i][j].position.y) < 40 && (ball.position.y - bubble[i][j].position.y) > 20) {

				if (j == COL - 1) { // En saðýn saðýna eklememesi için
					bubble[i + 1][j - 1].active = true;
					bubble[i + 1][j - 1].color = ball.color;
					lineCounter = i + 1;
					columnCounter = j - 1;
				}
				else {
					bubble[i + 1][j].active = true;
					bubble[i + 1][j].color = ball.color;
					lineCounter = i + 1;
					columnCounter = j;
				}

			}

			// Sað ortadaki topun temasý
			else if ((ball.position.y - bubble[i][j].position.y) <= 20 && (ball.position.y - bubble[i][j].position.y) > -20) {

				bubble[i][j + 1].active = true;
				bubble[i][j + 1].color = ball.color;
				lineCounter = i;
				columnCounter = j + 1;
			}

			// Sað üstekki topun temasý
			else if ((ball.position.y - bubble[i][j].position.y) <= -20 && (ball.position.y - bubble[i][j].position.y) > -40) {

				bubble[i - 1][j].active = true;
				bubble[i - 1][j].color = ball.color;
				lineCounter = i - 1;
				columnCounter = j;

			}

		}
		else {
			// Sol üstteki topun temasý
			if ((ball.position.y - bubble[i][j].position.y) < -20 && (ball.position.y - bubble[i][j].position.y) >= -40) {

				bubble[i - 1][j - 1].active = true;
				bubble[i - 1][j - 1].color = ball.color;
				lineCounter = i - 1;
				columnCounter = j - 1;

			}

			// Sol ortadaki topun temasý
			else if ((ball.position.y - bubble[i][j].position.y) < 20 && (ball.position.y - bubble[i][j].position.y) >= -20) {


				bubble[i][j - 1].active = true;
				bubble[i][j - 1].color = ball.color;

				lineCounter = i;
				columnCounter = j - 1;

			}

			// Sol alttaki topun temasý 
			else if ((ball.position.y - bubble[i][j].position.y) <= 40 && (ball.position.y - bubble[i][j].position.y) > 20) {

				if (!j == 0) {

					bubble[i + 1][j - 1].active = true;
					bubble[i + 1][j - 1].color = ball.color;
					lineCounter = i + 1;
					columnCounter = j - 1;
				}
				else { // En solun soluna eklememsi için
					bubble[i + 1][j].active = true;
					bubble[i + 1][j].color = ball.color;
					lineCounter = i + 1;
					columnCounter = j;
				}
			}
		}
	}
	else
	{
		if ((ball.position.x - bubble[i][j].position.x) > 0) {

			// Sað alttaki topun temasý 
			if ((ball.position.y - bubble[i][j].position.y) < 40 && (ball.position.y - bubble[i][j].position.y) >= 20) {

				bubble[i + 1][j + 1].active = true;
				bubble[i + 1][j + 1].color = ball.color;
				lineCounter = i + 1;
				columnCounter = j + 1;

			}


			// Sað ortadaki topun temasý
			else if ((ball.position.y - bubble[i][j].position.y) < 20 && (ball.position.y - bubble[i][j].position.y) >= -20) {

				bubble[i][j + 1].active = true;
				bubble[i][j + 1].color = ball.color;
				lineCounter = i;
				columnCounter = j + 1;
			}


			// Sað üstekki topun temasý
			else if ((ball.position.y - bubble[i][j].position.y) < -20 && (ball.position.y - bubble[i][j].position.y) > -40) {

				bubble[i - 1][j + 1].active = true;
				bubble[i - 1][j + 1].color = ball.color;
				lineCounter = i - 1;
				columnCounter = j + 1;

			}

		}
		else {
			// Sol üstteki topun temasý
			if ((ball.position.y - bubble[i][j].position.y) < -20 && (ball.position.y - bubble[i][j].position.y) >= -40) {

				bubble[i - 1][j].active = true;
				bubble[i - 1][j].color = ball.color;
				lineCounter = i - 1;
				columnCounter = j;
			}

			// Sol ortadaki topun temasý
			else if ((ball.position.y - bubble[i][j].position.y) < 20 && (ball.position.y - bubble[i][j].position.y) >= -20) {

				bubble[i][j - 1].active = true;
				bubble[i][j - 1].color = ball.color;
				lineCounter = i;
				columnCounter = j - 1;
			}

			// Sol alttaki topun temasý 
			else if ((ball.position.y - bubble[i][j].position.y) <= 40 && (ball.position.y - bubble[i][j].position.y) >= 20) {

				bubble[i + 1][j].active = true;
				bubble[i + 1][j].color = ball.color;
				lineCounter = i + 1;
				columnCounter = j;
			}
		}
	}

}
int ExplodeConnectedBubbles() {

	// Eðer en az 3 baðlý bubble varsa hepsini patlatýr
	int counter = 0;
	for (int i = 0; i < MAX_LINES; i++) {
		for (int j = 0; j < COL; j++) {
			if (bubble[i][j].exploded)
			{
				counter++;
			}

		}
	}

	// En az üç tane baðlý bubble varsa patlat
	if (counter >= 3)
	{

		for (int i = 0; i < MAX_LINES; i++) {
			for (int j = 0; j < COL; j++) {
				if (bubble[i][j].exploded)
				{
					ball.active = false;
					bubble[i][j].active = false;
					point += 10;
					PlaySound(ExplosionSound);
					ball.position = (Vector2){ ScreenWidth / 2, ScreenHeight - 50 };
					DrawGame();
					Sleep(75);

				}

			}
		}

		// Tüm exploded larý false yap
		for (int i = 0; i < MAX_LINES; i++) {
			for (int j = 0; j < COL; j++) {
				if (bubble[i][j].exploded)
				{
					bubble[i][j].exploded = false;
				}

			}
		}

		return 0;
	}
	else
	{
		for (int i = 0; i < MAX_LINES; i++) {
			for (int j = 0; j < COL; j++) {
				if (bubble[i][j].exploded)
				{
					bubble[i][j].exploded = false;
				}
			}
		}

		return 1;
	}
}
int CountConnectedBubbles(int i, int j, int color)
{
	// En az 3 tane baðlý bubble olup olmadýðýný kontrol eder
	if (i < 0 || i >= MAX_LINES || j < 0 || j >= COL || ColorToInt(bubble[i][j].color) != color || bubble[i][j].exploded || !bubble[i][j].active) {
		return 0;
	}
	bubble[i][j].exploded = true;

	// Çevresindeki bubble larý çaðýrýyor
	if (i % 2 == 0)
	{
		CountConnectedBubbles(i, j + 1, color);
		CountConnectedBubbles(i, j - 1, color);
		CountConnectedBubbles(i + 1, j, color);
		CountConnectedBubbles(i + 1, j - 1, color);
		CountConnectedBubbles(i - 1, j, color);
		CountConnectedBubbles(i - 1, j - 1, color);
	}
	else
	{
		CountConnectedBubbles(i, j + 1, color);
		CountConnectedBubbles(i, j - 1, color);
		CountConnectedBubbles(i + 1, j + 1, color);
		CountConnectedBubbles(i + 1, j, color);
		CountConnectedBubbles(i - 1, j + 1, color);
		CountConnectedBubbles(i - 1, j, color);
	}
}
int MarkAndExplodeConnectedBubbles(int i, int j)
{
	// Count ve explode fonksiyonlarýný tek bir fonksiyonda toplama 
	int color = ColorToInt(ball.color);
	CountConnectedBubbles(i, j, color);
	return  ExplodeConnectedBubbles();
}
void checkExtraExplode() {
	// Havada asýlý kalan toplarýn kontrolü (havada top varsa patlat)

	// Aktifliði false olan tüm bubble larýn exploded'ýný true yap
	for (int i = 0; i < MAX_LINES; i++) {
		for (int j = 0; j < COL; j++) {
			if (!bubble[i][j].active)
			{
				bubble[i][j].exploded = true;
			}
		}
	}


	// Havada izole þekilde ana kümeden ayrý bubble larý patlat

	for (int k = 0; k < 20; k++)
	{
		for (int i = 0; i < MAX_LINES; i++) {

			if (i % 2 == 0)
			{
				for (int j = 0; j < COL; j++) {
					if (i > 0 && j > 0 && j < COL && bubble[i][j].active && bubble[i][j - 1].exploded && bubble[i - 1][j - 1].exploded && bubble[i - 1][j].exploded)
					{

						bubble[i][j].exploded = true;

					}
				}
			}
			else
			{
				for (int j = 0; j < COL; j++) {
					if (i > 0 && j > 0 && j < COL && bubble[i][j].active && bubble[i][j - 1].exploded && bubble[i - 1][j].exploded && bubble[i - 1][j + 1].exploded)
					{

						bubble[i][j].exploded = true;

					}
				}
			}
		}
	}




	// Saðýndaki bubble active ise explode false olmalý

	for (int k = 0; k < 20; k++)
	{
		for (int i = 0; i < MAX_LINES; i++) {

			if (i % 2 == 0)
			{
				for (int j = COL; j > 0; j--) {
					if (i > 0 && bubble[i][j].active && bubble[i][j].exploded && bubble[i][j + 1].active && !bubble[i][j + 1].exploded)
					{

						bubble[i][j].exploded = false;

					}
				}
			}
			else
			{
				for (int j = COL; j > 0; j--) {
					if (i > 0 && bubble[i][j].active && bubble[i][j].exploded && bubble[i][j + 1].active && !bubble[i][j + 1].exploded)
					{

						bubble[i][j].exploded = false;

					}
				}
			}
		}
	}

	// Duvar kenarýnda olup ana kümede ayrý izole bubble larý patlatmak
	for (int k = 0; k < 40; k++)
	{
		for (int i = 0; i < MAX_LINES; i++) {

			if (i % 2 == 0)
			{

				for (int j = 0; j < COL; j++) {
					if (i > 0 && j == 0 && j < COL && bubble[i][j].active && bubble[i - 1][j].exploded)
					{

						bubble[i][j].exploded = true;


					}
				}

				for (int j = 0; j < COL; j++) {
					if (i > 0 && j == 19 && j < COL && bubble[i][j].active && bubble[i - 1][j - 1].exploded)
					{

						bubble[i][j].exploded = true;


					}
				}
			}
			else
			{
				for (int j = 0; j < COL; j++) {
					if (i > 0 && j == 0 && j < COL && bubble[i][j].active && bubble[i - 1][j].exploded && bubble[i - 1][j + 1].exploded)
					{

						bubble[i][j].exploded = true;


					}
				}

				for (int j = 0; j < COL; j++) {
					if (i > 0 && j == 18 && j < COL && bubble[i][j].active && bubble[i - 1][j].exploded && bubble[i - 1][j + 1].exploded)
					{

						bubble[i][j].exploded = true;


					}
				}
			}
		}
	}




	// Bubble active deðilse explode true olmak zorunda deðil
	for (int i = 0; i < MAX_LINES; i++) {
		for (int j = 0; j < COL; j++) {
			if (!bubble[i][j].active)
			{
				bubble[i][j].exploded = false;
			}

		}
	}

	// Explode true ise active false olacak
	for (int i = 0; i < MAX_LINES; i++) {
		for (int j = 0; j < COL; j++) {
			if (bubble[i][j].exploded)
			{
				ball.active = false;

				bubble[i][j].active = false;
				point += 15;
				PlaySound(ExplosionSound);
				Sleep(75);
				DrawGame();

			}
		}
	}




	// Explode true olanlar tekrardan false oluyor
	for (int i = 0; i < MAX_LINES; i++) {
		for (int j = 0; j < COL; j++) {
			if (bubble[i][j].exploded)
			{
				bubble[i][j].exploded = false;
			}

		}
	}



}
void UpdateGame() {

	if (!gameOver)
	{

		if (!ball.active) {
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
				PlaySound(ShotSound);
				// Topun kendi hizasýnýn 20 üstünden altýna atýlmamasý için
				if (ball.position.y - GetMousePosition().y <= 20)
					ball.active = false;
				else {
					ball.active = true;
					float constRatio = ((pow(GetMousePosition().x - ball.position.x, 2)) + (pow(GetMousePosition().y - ball.position.y, 2))) / 150;
					ball.speed.x = sqrt(pow(GetMousePosition().x - ball.position.x, 2) / constRatio);
					ball.speed.y = -sqrt(pow(GetMousePosition().y - ball.position.y, 2) / constRatio);

					if (GetMousePosition().x - ball.position.x < 0)
						ball.speed.x *= -1;
				}

			}
		}

		if (ball.active) {
			ball.position.x += ball.speed.x;
			ball.position.y += ball.speed.y;
		}
		else {
			ball.position = (Vector2){ ScreenWidth / 2, ScreenHeight - 50 };
			isBallHitted = false;
		}
		// Toplarýn duvara çarptýüýnda geri dönmesini kontrol eden algoritma
		if (ball.position.x + ball.radius >= ScreenWidth)
			ball.speed.x *= -1;
		if (ball.position.x - ball.radius <= 0)
			ball.speed.x *= -1;
		if (ball.position.y - ball.radius <= 0)
			ball.speed.y *= -1; 5;
		if (ball.position.y + ball.radius >= ScreenHeight)
			ball.speed.y *= -1;

		if (IsKeyPressed(KEY_SPACE)) {

			if (!ball.active)
			{
				Color holder = ball.color;
				ball.color = ball2.color;
				ball2.color = holder;
			}
		}
		// Yerleþtirme ve patlatma
		for (int i = MAX_LINES; i >= 0; i--) {
			for (int j = 0; j < COL; j++) {
				if (bubble[i][j].active) {
					if (CheckCollisionCircles(bubble[i][j].position, ball.radius, ball.position, ball.radius)) {
						ball.active = false;
						if (!isBallHitted)
						{
							Addition(i, j);

							if (MarkAndExplodeConnectedBubbles(lineCounter, columnCounter))
							{
								health--;
								ball.color = ball2.color;
								ball2.color = colorPalette[GetRandomValue(0, NUM_COLORS - 1)];

								isBallHitted = true;
							}
							else
							{
								ball.position = (Vector2){ ScreenWidth / 2, ScreenHeight - 50 };
								Sleep(50);

								ball.color = ball2.color;
								ball2.color = colorPalette[GetRandomValue(0, NUM_COLORS - 1)];

								isBallHitted = true;

								DrawGame();
								Sleep(200);

								for (int k = 0; k < 20; k++)
								{
									checkExtraExplode();
								}


								DrawGame();

							}
						}
					}
				}
			}
		}

		// Kaydýrma algoritmasý
		if (health == 0)
		{

			for (int i = MAX_LINES; i >= 0; i--) {
				if (i % 2 == 0)
				{
					for (int j = 0; j < COL; j++)
					{

						bubble[i + 2][j].active = bubble[i][j].active;
						bubble[i + 2][j].color = bubble[i][j].color;

					}
				}
				else
				{
					for (int j = 0; j < COL - 1; j++)
					{

						bubble[i + 2][j].active = bubble[i][j].active;
						bubble[i + 2][j].color = bubble[i][j].color;

					}
				}

			}
			for (int k = 0; k < COL; k++)
			{
				bubble[0][k].active = true;
				bubble[0][k].color = colorPalette[GetRandomValue(0, NUM_COLORS - 1)];

			}
			for (int k = 0; k < COL - 1; k++)
			{

				bubble[1][k].active = true;
				bubble[1][k].color = colorPalette[GetRandomValue(0, NUM_COLORS - 1)];
			}
			health = MAX_HEALTH;

		}

		// Oyun bitiþ kontrolü

		for (int j = 0; j < COL; j++) {
			if (bubble[16][j].active)
			{
				if (highPoint < point) {
					highPoint = point;
				}
				gameOver = true;
			}

		}
	}
	// Oyunu yeniden baþlatma
	else
	{
		if (IsKeyPressed(KEY_ENTER))
		{
			InitGame();
			gameOver = false;
		}
	}

}
void DrawGame() {

	BeginDrawing();
	ClearBackground(RAYWHITE);
	if (!gameOver)
	{
		DrawTexture(arkaPlan, 0, 0, WHITE);
		// Finish Line
		for (int k = 0; k < 100; k++) {
			DrawRectangle(40 * k + 5, ScreenHeight - 80, 30, 3, BLACK);
		}

		// Bubble larý çizdirmek
		for (int i = 0; i < MAX_LINES; i++) {
			for (int j = 0; j < COL; j++) {
				if (bubble[i][j].active) {
					DrawTexture(bubble[i][j].texture, bubble[i][j].position.x - 23.5, bubble[i][j].position.y - 23.5, bubble[i][j].color);
				}

			}
		}
		if (GetMousePosition().y - ball.position.y < -20)
		{
			Vector2 positiond = { ScreenWidth / 2, ScreenHeight - 47 };
			DrawLineEx(positiond, GetMousePosition(), 5, LIGHTGRAY);
		}

		// Ball ve Ball2 çizdirme
		Vector2 nposition = { ball.position.x - 27, ball.position.y - 15 };
		Vector2 n2position = { ball2.position.x - 33, ball2.position.y - 15 };
		DrawTextureEx(ball.texture, nposition, 0, 1.0f, ball.color);
		DrawTextureEx(ball2.texture, n2position, 0, 1.0f, ball2.color);

		// Health Barý
		DrawText("HEALTH", 32, ScreenHeight - 60, 25, BLACK);
		for (int i = 0; i < health; i++)
		{
			DrawRectangle(25 + i * 40, ScreenHeight - 30, 35, 10, RED);
		}

		// Point barý
		DrawText(TextFormat("POINT = %d", point), ScreenWidth - 200, ScreenHeight - 30, 25, BLACK);



	}

	else
	{
		DrawTexture(arkaPlan, 0, 0, WHITE);
		// Finish Line
		for (int k = 0; k < 100; k++) {
			DrawRectangle(40 * k + 5, ScreenHeight - 80, 30, 3, BLACK);
		}

		for (int i = 0; i < MAX_LINES; i++) {
			for (int j = 0; j < COL; j++) {
				if (bubble[i][j].active) {
					DrawTexture(bubble[i][j].texture, bubble[i][j].position.x - 23.5, bubble[i][j].position.y - 23.5, bubble[i][j].color);
				}

			}
		}

		// Health Barý
		DrawText("HEALTH", 32, ScreenHeight - 60, 25, DARKGRAY);
		for (int i = 0; i < health; i++)
		{
			DrawRectangle(25 + i * 40, ScreenHeight - 30, 35, 10, RED);
		}
		DrawRectangle(GetScreenWidth() / 2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20) / 2 - 85, 110, 470, 470, BLACK);
		// Background
		DrawRectangle(GetScreenWidth() / 2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20) / 2 - 75, 120, 450, 450, WHITE);
		// Point barý
		DrawText(TextFormat("POINT = %d", point), ScreenWidth - 200, ScreenHeight - 60, 25, DARKGRAY);

		DrawText("GAME OVER", GetScreenWidth() / 2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20) / 2, ScreenHeight / 2 - 180, 50, GRAY);
		DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth() / 2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20) / 2 - 10, ScreenHeight / 2 + 160, 20, GRAY);
		DrawText("PRESS [B] TO BACK TO THE MENU", GetScreenWidth() / 2 - MeasureText("PRESS [B] TO BACK TO THE MENU", 20) / 2 - 10, ScreenHeight / 2 + 120, 20, GRAY);
		DrawText(TextFormat("SCORE = %d", point), ScreenWidth / 2 - 100, ScreenHeight / 2 - 50, 30, DARKGRAY);
		DrawText(TextFormat("HIGH SCORE = %d", highPoint), ScreenWidth / 2 - 135, ScreenHeight / 2 + 30, 30, DARKGRAY);


	}

	EndDrawing();
}
void UpdateMenu() {
	if (IsKeyPressed(KEY_UP)) {
		selectedMenuItemIndex--;
		if (selectedMenuItemIndex < 0) {
			selectedMenuItemIndex = numberOfMenuItems - 1;
		}
	}
	else if (IsKeyPressed(KEY_DOWN)) {
		selectedMenuItemIndex++;
		if (selectedMenuItemIndex >= numberOfMenuItems) {
			selectedMenuItemIndex = 0;
		}
	}
	if (IsKeyPressed(KEY_ENTER)) {
		switch (selectedMenuItemIndex) {
		case 0:
			gameState = GAME;
			break;
		case 1:
			gameState = HOW_TO_PLAY;
			break;
		case 2:
			gameState = FINISH;
			break;
		}
	}

}
void DrawMenu() {
	BeginDrawing();
	ClearBackground(RAYWHITE);
	// Menünün çizilmesi
	DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), GRAY);

	// Menü baþlýðýný çiz
	DrawText("BUBBLE SHOOTER", GetScreenWidth() / 2 - MeasureText("BUBBLE SHOOTER", 70) / 2, 80, 70, BLACK);

	// Menü ögelerini çiz
	DrawText("Play", GetScreenWidth() / 2 - MeasureText("Play", 50) / 2, 280, 50, BLACK);
	DrawText("How to Play", GetScreenWidth() / 2 - MeasureText("How to Play", 50) / 2, 400, 50, BLACK);
	DrawText("Exit", GetScreenWidth() / 2 - MeasureText("Exit", 50) / 2, 520, 50, BLACK);
	DrawRectangleLines(GetScreenWidth() / 2 - rectangleHeight / 2, 270, rectangleHeight, rectangleWidth, BLACK);
	DrawRectangleLines(GetScreenWidth() / 2 - rectangleHeight / 2, 390, rectangleHeight, rectangleWidth, BLACK);
	DrawRectangleLines(GetScreenWidth() / 2 - rectangleHeight / 2, 510, rectangleHeight, rectangleWidth, BLACK);

	int menuItemHeight = 50;
	int Posy = 270 + selectedMenuItemIndex * 120;
	DrawRectangleLines(GetScreenWidth() / 2 - rectangleHeight / 2, Posy, rectangleHeight, rectangleWidth, YELLOW);
	EndDrawing();
}
void UnloadGame() {

	// Ses dosyalarýný serbest býrak
	UnloadSound(ExplosionSound);
	UnloadSound(ShotSound);

	for (int i = 0; i < MAX_LINES; i++) {
		for (int j = 0; j < COL; j++) {
			UnloadTexture(bubble[i][j].texture);
		}
	}
	UnloadTexture(ball.texture);
	UnloadTexture(ball2.texture);
	CloseAudioDevice();
}