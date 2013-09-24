#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <allegro.h>

sem_t agente, fosforo, papel, tabaco, fosforo_sem, papel_sem, tabaco_sem;

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
    printf("T: %c P: %c F: %c\n", letra_estado(Tabaco), letra_estado(Papel), 
letra_estado(Fosforo));
}

void *agente_a(void *id) {
    while (1) {
        sem_wait(&agente);
        
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
        
        sem_post(&agente);
        
        // Ele fuma
        pthread_mutex_lock(&global);
        estado_global[Tabaco] = Fumando;
        imprime_estado_global();
        pthread_mutex_unlock(&global);
        
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
        
        sem_post(&agente);
        
        // Ele fuma
        pthread_mutex_lock(&global);
        estado_global[Papel] = Fumando;
        imprime_estado_global();
        pthread_mutex_unlock(&global);
        
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
        
        sem_post(&agente);
        
        // Ele fuma
        pthread_mutex_lock(&global);
        estado_global[Fosforo] = Fumando;
        imprime_estado_global();
        pthread_mutex_unlock(&global);
        
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

int main(void) {
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

    agente_b = load_bmp("bitmaps/agente_b.bmp", NULL);
    smoking = load_bmp("bitmaps/smoking.bmp", NULL);
    smoking_not = load_bmp("bitmaps/smoking_not.bmp", NULL);
    matches = load_bmp("bitmaps/matches.bmp", NULL);
    paper = load_bmp("bitmaps/paper.bmp", NULL);
    tabaco_b = load_bmp("bitmaps/tobacco.bmp", NULL);
    gear = load_bmp("bitmaps/gear.bmp", NULL);
    
    while (!key[KEY_ESC]) {
        BITMAP *buffer = create_bitmap(SCREEN_W, SCREEN_H);
        
        pthread_mutex_lock(&global);
        
        /* Desenha os fumantes */
        
        /* Desenha o Agente*/
        
        /* Desenha os ingredientes disponiveis */
        
        /* Fim dos desenhos */
        
        pthread_mutex_unlock(&global);
        
        blit(buffer, screen, 0, 0, 0, 0, buffer->w, buffer->h);
        
        ++timestamp;
        
        timestamp = timestamp % 32000;
        
        rest(20);
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