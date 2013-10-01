#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <allegro.h>

#define SLEEP_FUMA (10)
#define SLEEP_FAZ (5)
#define SLEEP_INGREDIENTES (10)

sem_t agente, fosforo, papel, tabaco, fosforo_sem, papel_sem, tabaco_sem, anima_sem;

pthread_mutex_t mutex, global;

int isTobacco, isPaper, isMatch, allegro_fim;

typedef enum { Esperando, Fazendo, Fumando } estados;

typedef enum { Tabaco = 0, Papel = 1, Fosforo = 2 } fumantes;

estados estado_global[3];

estados disponivel[3];

char letra_estado(fumantes f) {
    switch (estado_global[f]) {
        case Esperando:
            return 'E';
            break;
        case Fazendo:
            return 'f';
            break;
        case Fumando:
            return 'F';
            break;
        default:
            return 'R';
            break;
    }
}

void imprime_estado_global() {
    printf("T: %c P: %c F: %c %d %d %d\n", letra_estado(Tabaco), 
letra_estado(Papel), letra_estado(Fosforo), disponivel[0], disponivel[1], 
disponivel[2]);
}

void *agente_a(void *id) {
    while (1) {
        sem_wait(&agente);
		sem_wait(&anima_sem);        
		
		sleep(SLEEP_INGREDIENTES);

        pthread_mutex_lock(&global);
        printf("Liberou fosforo e papel!\n");
        disponivel[Tabaco] = 0;
        disponivel[Papel] = 1;
        disponivel[Fosforo] = 1;
        
        imprime_estado_global();
        pthread_mutex_unlock(&global);
        
        sem_post(&fosforo);
        sem_post(&papel);
    }
    
    return NULL;
}

void *agente_b(void *id) {
    while (1) {
        sem_wait(&agente);
	    sem_wait(&anima_sem);
        
        sleep(SLEEP_INGREDIENTES);
        
        pthread_mutex_lock(&global);
        printf("Liberou fosforo e tabaco!\n");
        disponivel[Tabaco] = 1;
        disponivel[Papel] = 0;
        disponivel[Fosforo] = 1;
        
        imprime_estado_global();
        pthread_mutex_unlock(&global);
        
        sem_post(&fosforo);
        sem_post(&tabaco);
    }
    
    return NULL;
}

void *agente_c(void *id) {
    while (1) {
        sem_wait(&agente);
	    sem_wait(&anima_sem);
	    
    	sleep(SLEEP_INGREDIENTES);
        
        pthread_mutex_lock(&global);
        printf("Liberou papel e tabaco!\n");
        disponivel[Tabaco] = 1;
        disponivel[Papel] = 1;
        disponivel[Fosforo] = 0;
        
        imprime_estado_global();
        pthread_mutex_unlock(&global);
        
        sem_post(&papel);
        sem_post(&tabaco);
    }
    
    return NULL;
}

// tem tabaco
void *fumante_a(void *id) {
    while (allegro_fim == 0) {
        sem_wait(&tabaco_sem);
        
        // Ele faz cigarro
        pthread_mutex_lock(&global);
        estado_global[Tabaco] = Fazendo;
        imprime_estado_global();
        pthread_mutex_unlock(&global);
        sleep(SLEEP_FAZ);
        
        sem_post(&agente);
        
        // Ele fuma
        pthread_mutex_lock(&global);
        estado_global[Tabaco] = Fumando;
        imprime_estado_global();
        pthread_mutex_unlock(&global);
        sleep(SLEEP_FUMA);
        
        // Ele acaba de fumar, agora espera
        pthread_mutex_lock(&global);
        estado_global[Tabaco] = Esperando;
        imprime_estado_global();
        pthread_mutex_unlock(&global);
    }
    
    return NULL;
}

// tem papel
void *fumante_b(void *id) {
    while (allegro_fim == 0) {
        sem_wait(&papel_sem);
        
        // Ele faz cigarro
        pthread_mutex_lock(&global);
        estado_global[Papel] = Fazendo;
        imprime_estado_global();
        pthread_mutex_unlock(&global);
        sleep(SLEEP_FAZ);
        
        sem_post(&agente);
        
        // Ele fuma
        pthread_mutex_lock(&global);
        estado_global[Papel] = Fumando;
        imprime_estado_global();
        pthread_mutex_unlock(&global);
        sleep(SLEEP_FUMA);
        
        // Ele acaba de fumar, agora espera
        pthread_mutex_lock(&global);
        estado_global[Papel] = Esperando;
        imprime_estado_global();
        pthread_mutex_unlock(&global);
    }
    
    return NULL;
}

// tem fosforo
void *fumante_c(void *id) {
    while (allegro_fim == 0) {
        sem_wait(&fosforo_sem);
        
        // Ele faz cigarro
        pthread_mutex_lock(&global);
        estado_global[Fosforo] = Fazendo;
        imprime_estado_global();
        pthread_mutex_unlock(&global);
        sleep(SLEEP_FAZ);
        
        sem_post(&agente);
        
        // Ele fuma
        pthread_mutex_lock(&global);
        estado_global[Fosforo] = Fumando;
        imprime_estado_global();
        pthread_mutex_unlock(&global);
        sleep(SLEEP_FUMA);
        
        // Ele acaba de fumar, agora espera
        pthread_mutex_lock(&global);
        estado_global[Fosforo] = Esperando;
        imprime_estado_global();
        pthread_mutex_unlock(&global);
    }
    
    return NULL;
}

// Puxa o tabaco
void *pusher_a(void *id) {
    while (1) {
        sem_wait(&tabaco);
        pthread_mutex_lock(&mutex);
        
        if (isPaper == 1) {
            isPaper = 0;
            sem_post(&fosforo_sem);
        } else if (isMatch == 1) {
            isMatch = 0;
            sem_post(&papel_sem);
        } else {
            isTobacco = 1;
        }    
        pthread_mutex_unlock(&mutex);
    }
    
    return NULL;
}


// Puxa o papel
void *pusher_b(void *id) {
    while (1) {
        sem_wait(&papel);
        pthread_mutex_lock(&mutex);
        
        if (isTobacco == 1) {
            isTobacco = 0;
            sem_post(&fosforo_sem);
        } else if (isMatch == 1) {
            isMatch = 0;
            sem_post(&tabaco_sem);
        } else {
            isPaper = 1;
        }    
        pthread_mutex_unlock(&mutex);
    }
    
    return NULL;
}

// Puxa o fosforo
void *pusher_c(void *id) {
    while (1) {
        sem_wait(&fosforo);
        pthread_mutex_lock(&mutex);
        
        if (isPaper == 1) {
            isPaper = 0;
            sem_post(&tabaco_sem);
        } else if (isTobacco == 1) {
            isTobacco = 0;
            sem_post(&papel_sem);
        } else {
            isMatch = 1;
        }    
        pthread_mutex_unlock(&mutex);
    }
    
    return NULL;
}

void callback() {
    if (key[KEY_ESC]) {
        pthread_mutex_destroy(&mutex);
        pthread_mutex_destroy(&global);
        
        sem_destroy(&agente);
        sem_destroy(&fosforo);
        sem_destroy(&papel);
        sem_destroy(&tabaco);
        sem_destroy(&fosforo_sem);
        sem_destroy(&papel_sem);
        sem_destroy(&tabaco_sem);
        
        exit(0);
    }
}

void desenha_fumante(BITMAP *buffer, BITMAP *smoking, BITMAP *smoking_not,
BITMAP *gear, int fumante, int passos, int passos_total, int offset) {
    if (estado_global[fumante] == Fazendo) {
        int x = 550;
        int y = offset;
        BITMAP *tmp = create_bitmap(SCREEN_W, SCREEN_H);
        
        blit(buffer, tmp, 0, 0, 0, 0, buffer->w, buffer->h);
        
        fixed ang;
        
        // Anda até o centro
        for (passos = 0; passos < passos_total; ++passos, x-=10) {
            blit(tmp, buffer, 0, 0, 0, 0, buffer->w, buffer->h);
            
            draw_sprite(buffer, smoking_not, x, y);
                        
            blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
            rest_callback(75, callback);
        }
        
        // Faz o cigarro
        for (ang = 0, passos = 0; passos < passos_total*3; ang += 37, ++passos) {
            blit(tmp, buffer, 0, 0, 0, 0, buffer->w, buffer->h);
            
            draw_sprite(buffer, smoking_not, x, y);
            rotate_sprite(buffer, gear, x-50, y+50, itofix(ang));
                        
            blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
            rest_callback(40, callback);        
        }
     
     
        // Volta a posicao
        for (passos = 0; passos < passos_total; ++passos, x+=10) {
            blit(tmp, buffer, 0, 0, 0, 0, buffer->w, buffer->h);
            
            draw_sprite(buffer, smoking_not, x, y);
                        
            blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
            rest_callback(75, callback);
        }  
    }
}

int main(void) {
    int cig;

    printf("Quantos cigarros poderam ser feitos?\n"); 
    scanf("%d", &cig);

    pthread_t t_agentea, t_agenteb, t_agentec, t_fumantea, t_fumanteb, 
t_fumantec, t_pushera, t_pusherb, t_pusherc;
    
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&global, NULL);
    
    sem_init(&agente, 0, 1);
    sem_init(&fosforo, 0, 0);
    sem_init(&papel, 0, 0);
    sem_init(&tabaco, 0, 0);
    sem_init(&fosforo_sem, 0, 0);
    sem_init(&papel_sem, 0, 0);
    sem_init(&tabaco_sem, 0, 0);
    sem_init(&anima_sem, 0, 0);

    int count;

    if (cig > 0) {
        for (count = 0; count < cig; ++count) sem_post(&anima_sem);
    } else sem_post(&anima_sem);

    isTobacco = isPaper = isMatch = 0;
    
    pthread_create(&t_agentea, NULL, agente_a, NULL);
    pthread_create(&t_agenteb, NULL, agente_b, NULL);
    pthread_create(&t_agentec, NULL, agente_c, NULL);
    pthread_create(&t_fumantea, NULL, fumante_a, NULL);
    pthread_create(&t_fumanteb, NULL, fumante_b, NULL);
    pthread_create(&t_fumantec, NULL, fumante_c, NULL);
    pthread_create(&t_pushera, NULL, pusher_a, NULL);
    pthread_create(&t_pusherb, NULL, pusher_b, NULL);
    pthread_create(&t_pusherc, NULL, pusher_c, NULL);
    
    allegro_init();
    
    install_timer();
    install_keyboard();
    
    allegro_fim = 0;
    
    set_color_depth(32);
    
    if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0) != 0) {
        //abort_on_error("Couldn't set a 32 bit color resolution");
    }
    
    set_window_title("Problema dos Fumantes - Pressione ESC para sair");
    
    int x = 0, y = 0, timestamp = 0;
    
    BITMAP *agente_b, *smoking, *smoking_not, *matches, 
*paper, *tabaco_b, *gear;

    agente_b = load_bmp("bitmaps/agent_b.bmp", NULL);
    smoking = load_bmp("bitmaps/smoking.bmp", NULL);
    smoking_not = load_bmp("bitmaps/smoking_not.bmp", NULL);
    matches = load_bmp("bitmaps/matches.bmp", NULL);
    paper = load_bmp("bitmaps/paper.bmp", NULL);
    tabaco_b = load_bmp("bitmaps/tobacco.bmp", NULL);
    gear = load_bmp("bitmaps/gear.bmp", NULL);
    
    int passos, passos_total = 27;
    
    while (!key[KEY_ESC]) {
        BITMAP *buffer = create_bitmap(SCREEN_W, SCREEN_H);
        
        pthread_mutex_lock(&global);

	rectfill(buffer, 0, 0, SCREEN_W, SCREEN_H, makecol(255, 255, 255));
        
        draw_sprite(buffer, tabaco_b, 490, 90);
        draw_sprite(buffer, paper, 490, 230);
        draw_sprite(buffer, matches, 490, 390);

	//textprintf_ex(buffer, font, 10, 10, makecol(255, 100, 200),
	//	    -1, "Tempo: %d", timestamp/20);
		

	
        
        //printf("ALLEGRO\n");
        //imprime_estado_global();  
        
        int offsets[3];
        
        offsets[0] = 10;
        offsets[1] = 180;
        offsets[2] = 330;
        
        for (x = 0; x < 3; ++x) {
            if (estado_global[x] == Esperando) {
                draw_sprite(buffer, smoking_not, 550, offsets[x]);
            } else if (estado_global[x] == Fumando) {
                draw_sprite(buffer, smoking, 550, offsets[x]);
            }       
        }

        
        /* Desenha o Agente*/
        draw_sprite(buffer, agente_b, 0, 135);
        
        /* Desenha os ingredientes disponiveis */
        if (disponivel[Tabaco] == 1) {
            draw_sprite(buffer, tabaco_b, 150, 175+0);
        }
        
        if (disponivel[Fosforo] == 1) {
            draw_sprite(buffer, matches, 150, 175+35);
        }
        
        if (disponivel[Papel] == 1) {
            draw_sprite(buffer, paper, 150, 175+90);
        }
        
        	/* Anima os fumantes */
        desenha_fumante(buffer, smoking, smoking_not, gear, Tabaco, passos, passos_total, 10);
        desenha_fumante(buffer, smoking, smoking_not, gear, Papel, passos, passos_total, 180);
        desenha_fumante(buffer, smoking, smoking_not, gear, Fosforo, passos, passos_total, 330);
        
        /* Fim dos desenhos */
        
        pthread_mutex_unlock(&global);
        
        blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
        
        ++timestamp;
        
        timestamp = timestamp % 32000;
        
        rest_callback(50, callback);
    }
    
    allegro_fim = 1;
    
    pthread_join(t_fumantea, NULL);
    pthread_join(t_fumanteb, NULL);
    pthread_join(t_fumantec, NULL);
    
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&global);
    
    sem_destroy(&agente);
    sem_destroy(&fosforo);
    sem_destroy(&papel);
    sem_destroy(&tabaco);
    sem_destroy(&fosforo_sem);
    sem_destroy(&papel_sem);
    sem_destroy(&tabaco_sem);
    
    return 0;
}
//END_OF_MAIN()
