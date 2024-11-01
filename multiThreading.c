#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

sem_t mutex;
int array[10000][2];
int topNodes[50];
int randomNodes[50];
int partitions[58228];
int done = 0;

// Define a structure to represent a node in the graph
struct Node
{
    int data;
    struct Node *next;
};

// Define a structure to represent the graph
struct Graph
{
    int numVertices;
    struct Node **adjacencyList;
};

// Function to create a new node
struct Node *createNode(int data)
{
    struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

// Function to create a graph
struct Graph *createGraph(int numVertices)
{
    struct Graph *graph = (struct Graph *)malloc(sizeof(struct Graph));
    graph->numVertices = numVertices;
    graph->adjacencyList = (struct Node **)malloc(numVertices * sizeof(struct Node *));

    for (int i = 0; i < numVertices; i++)
    {
        graph->adjacencyList[i] = NULL;
    }

    return graph;
}

// Function to add an edge to the graph
void addEdge(struct Graph *graph, int src, int dest)
{
    struct Node *newNode = createNode(dest);
    newNode->next = graph->adjacencyList[src];
    graph->adjacencyList[src] = newNode;
}

// Function to print the graph
void printGraph(struct Graph *graph)
{
    for (int i = 0; i < graph->numVertices; i++)
    {
        struct Node *currentNode = graph->adjacencyList[i];
        printf("Node %d is connected to:", i);
        while (currentNode != NULL)
        {
            printf(" %d", currentNode->data);
            currentNode = currentNode->next;
        }
        printf("\n");
    }
}

int degree(struct Graph *graph, int node)
{
    int degree = 0;
    struct Node *currentNode = graph->adjacencyList[node];
    while (currentNode != NULL)
    {
        degree++;
        currentNode = currentNode->next;
    }
    return degree;
}

// Function to find the 50 nodes with the highest degree
void findTopDegreeNodes(struct Graph *graph, int *topNodes, int k)
{
    int *degrees = (int *)malloc(graph->numVertices * sizeof(int));

    // Initialize degrees
    for (int i = 0; i < graph->numVertices; i++)
    {
        degrees[i] = degree(graph, i);
    }

    // Find the top k nodes
    for (int i = 0; i < k; i++)
    {
        int maxDegree = -1;
        int maxNode = -1;
        for (int j = 0; j < graph->numVertices; j++)
        {
            if (degrees[j] > maxDegree)
            {
                maxDegree = degrees[j];
                maxNode = j;
            }
        }
        topNodes[i] = maxNode;
        degrees[maxNode] = -1; // Mark this node as processed
    }

    free(degrees);
}

// Function to remove an edge from the graph
void removeEdge(struct Graph *graph, int src, int dest)
{
    struct Node *currentNode = graph->adjacencyList[src];
    struct Node *prevNode = NULL;

    while (currentNode != NULL && currentNode->data != dest)
    {
        prevNode = currentNode;
        currentNode = currentNode->next;
    }

    if (currentNode != NULL)
    {
        if (prevNode == NULL)
        {
            graph->adjacencyList[src] = currentNode->next;
        }
        else
        {
            prevNode->next = currentNode->next;
        }
        free(currentNode);
    }
}

void updateGraph(struct Graph *graph)
{
    // Randomly decide whether to remove or add an edge
    int decision = rand() % (9 - 0 + 1) + 0;

    int node0 = rand() % graph->numVertices;
    int node1 = rand() % graph->numVertices;

    // Ensure that node0 and node1 are not the same
    while (node0 == node1)
    {
        node1 = rand() % graph->numVertices;
    }

    sem_wait(&mutex);
    printf("Updating...%d\n", done);
    FILE *fptr = fopen("update.log", "a");
    if (fptr == NULL)
    {
        perror("Error opening file");
        return;
    }

    if (decision < 8)
    {
        // Remove an edge
        removeEdge(graph, node0, node1);
        removeEdge(graph, node1, node0);
        fprintf(fptr, "REMOVED %d %d %ld\n", node0, node1, time(NULL));
    }
    else
    {
        // Add an edge
        // Check if the edge already exists before adding it
        fprintf(fptr, "ADDED %d %d %ld\n", node0, node1, time(NULL));
        if (graph->adjacencyList[node0] != NULL)
        {
            struct Node *currentNode = graph->adjacencyList[node0];
            while (currentNode != NULL)
            {
                if (currentNode->data == node1)
                {
                    printf("Updated\n");
                    sem_post(&mutex);
                    fclose(fptr);
                    return; // Edge already exists
                }
                currentNode = currentNode->next;
            }
        }

        addEdge(graph, node0, node1);
        addEdge(graph, node1, node0);
    }
    fclose(fptr);
    printf("Updated\n");
    sem_post(&mutex);
}

void *graphUpdateThread(void *arg)
{
    struct Graph *graph = (struct Graph *)arg;

    while (done < 10000)
    {
        // printf("2\n");
        updateGraph(graph);
        usleep(1000);
    }

    return NULL;
}

struct Queue
{
    struct Node *front;
    struct Node *rear;
};

void enqueue(struct Queue *q, int data)
{
    struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
    newNode->data = data;
    newNode->next = NULL;
    if (q->rear == NULL)
    {
        q->front = q->rear = newNode;
        return;
    }
    q->rear->next = newNode;
    q->rear = newNode;
}

int dequeue(struct Queue *q)
{
    if (q->front == NULL)
    {
        return -1; // Queue is empty
    }
    int data = q->front->data;
    struct Node *temp = q->front;
    q->front = q->front->next;
    free(temp);
    if (q->front == NULL)
    {
        q->rear = NULL;
    }
    return data;
}

bool isEmpty(struct Queue *q)
{
    return q->front == NULL;
}

int BFS(struct Graph *graph, int start, int end, int *parent)
{
    bool *visited = (bool *)malloc(58228 * sizeof(bool));
    for (int i = 0; i < 58228; i++)
    {
        visited[i] = false;
        parent[i] = -1;
    }

    struct Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));
    queue->front = queue->rear = NULL;

    enqueue(queue, start);
    visited[start] = true;

    while (!isEmpty(queue))
    {
        int current = dequeue(queue);
        struct Node *temp = graph->adjacencyList[current];

        while (temp)
        {
            int neighbor = temp->data;
            if (!visited[neighbor])
            {
                enqueue(queue, neighbor);
                visited[neighbor] = true;
                parent[neighbor] = current;

                if (neighbor == end)
                {
                    free(visited);
                    free(queue);
                    return 1; // Path found
                }
            }
            temp = temp->next;
        }
    }

    free(visited);
    free(queue);
    return 0; // No path found
}

void printShortestPath(int *parent, int start, int end, FILE *fptr, struct Graph *graph)
{
    if (start == end)
    {
        fprintf(fptr, "%d", start);
    }
    else if (parent[end] == -1)
    {
        fprintf(fptr, "No path found");
    }
    else
    {
        printShortestPath(parent, start, parent[end], fptr, graph);
        fprintf(fptr, " -> %d", end);
        removeEdge(graph, end, parent[end]);
        removeEdge(graph, parent[end], end);
    }
}

void *pathStitcherThread(void *arg)
{
    struct Graph *graph = (struct Graph *)arg;

    while (done < 10000)
    {
        sem_wait(&mutex);
        if (done < 10000)
        {
            int source = array[done][0];
            int destination = array[done][1];
            done++;

            printf("Path stitcher is searching for a path between %d and %d\n", source, destination);

            int parent1[58228];
            int parent2[58228];
            int parent3[58228];

            for (int i = 0; i < 58228; i++)
            {
                parent1[i] = -1;
                parent2[i] = -1;
                parent3[i] = -1;
            }

            int a = partitions[source];
            int b = partitions[destination];

            if (a >= 50)
            {
                a = a - 50;
            }
            if (b >= 50)
            {
                b = b - 50;
            }

            FILE *fptr = fopen("path_found.log", "a");
            if (fptr == NULL)
            {
                perror("Error opening file");
                sem_post(&mutex);
                return NULL;
            }

            if (BFS(graph, source, topNodes[a], parent1) && BFS(graph, topNodes[a], topNodes[b], parent2) && BFS(graph, topNodes[b], destination, parent3))
            {

                fprintf(fptr, "PATH FOUND AND REMOVED <%d, %d>: ", source, destination);
                printShortestPath(parent1, source, topNodes[a], fptr, graph);
                fprintf(fptr, " -> ");
                printShortestPath(parent2, topNodes[a], topNodes[b], fptr, graph);
                fprintf(fptr, " -> ");
                printShortestPath(parent3, topNodes[b], destination, fptr, graph);
                fprintf(fptr, " <%ld>\n", time(NULL));
            }
            else
            {
                fprintf(fptr, "No path found<%d, %d>\n", source, destination);
            }
            fclose(fptr);
        }
        sem_post(&mutex);
        usleep(1500); // Sleep for a while before searching for the next path
    }

    return NULL;
}

int main()
{
    int numVertices = 58228; // Number of vertices in the graph
    struct Graph *graph = createGraph(numVertices);

    FILE *file = fopen("loc-brightkite_edges.txt", "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    int num1, num2;
    while (fscanf(file, "%d %d", &num1, &num2) == 2)
    {
        addEdge(graph, num1, num2);
    }

    fclose(file);

    file = fopen("path_to_find.log", "w");
    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    // Seed the random number generator with the current time

    srand(time(NULL));

    for (int i = 0; i < 10000; i++)
    {
        int source = rand() % (58227 - 0 + 1) + 0;
        int destination = rand() % (58227 - 0 + 1) + 0;
        fprintf(file, "%d\t%d\n", source, destination);
    }

    fclose(file);
    printf("Generated 10000 random pairs of nodes and saved to 'path_to_find.log'\n");

    findTopDegreeNodes(graph, topNodes, 50);

    printf("Top 50 nodes with the highest degree:\n");
    for (int i = 0; i < 50; i++)
    {
        printf("Node %d, Degree: %d\n", topNodes[i], degree(graph, topNodes[i]));
    }

    int k = 0;

    while (k < 50)
    {
        int isFound = 0;
        int temp = rand() % (58227 - 0 + 1) + 0;
        for (int i = 0; i < 50; i++)
        {
            if (topNodes[i] == temp)
            {
                isFound = 1;
            }
        }
        for (int i = 0; i < k; i++)
        {
            if (randomNodes[i] == temp)
            {
                isFound = 1;
            }
        }
        if (isFound == 0)
        {
            randomNodes[k++] = temp;
        }
    }

    printf("Random 50 nodes\n");
    for (int i = 0; i < 50; i++)
    {
        printf("Random Node %d\n", randomNodes[i]);
    }

    for (int i = 0; i < numVertices; i++)
    {
        partitions[i] = rand() % (99 - 0 + 1) + 0;
    }

    file = fopen("landmark.log", "w");
    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }
    for (int i = 0; i < 100; i++)
    {
        if (i < 50)
        {
            fprintf(file, "Landmark Node %d: ", topNodes[i]);
        }
        else
        {
            fprintf(file, "Landmark Node %d: ", randomNodes[i - 50]);
        }
        for (int j = 0; j < numVertices; j++)
        {
            if (partitions[j] == i)
            {
                fprintf(file, "Node %d, ", j);
            }
        }
        fprintf(file, "\n");
    }
    fclose(file);

    file = fopen("path_to_find.log", "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    // Read integers from the file into the array
    for (int i = 0; i < 10000; i++)
    {
        fscanf(file, "%d\t%d", &array[i][0], &array[i][1]);
    }

    fclose(file);

    sem_init(&mutex, 0, 1);
    pthread_t graphUpdateThreads[5];
    pthread_t pathStitcherThreads[5];

    for (int i = 0; i < 5; i++)
    {
        if (pthread_create(&graphUpdateThreads[i], NULL, graphUpdateThread, (void *)graph) != 0)
        {
            perror("Error creating graph_update thread");
            return 1;
        }
    }
    for (int i = 0; i < 10; i++)
    {
        if (pthread_create(&pathStitcherThreads[i], NULL, pathStitcherThread, (void *)graph) != 0)
        {
            perror("Error creating path_stitcher thread");
            return 1;
        }
    }

    for (int i = 0; i < 5; i++)
    {
        pthread_join(graphUpdateThreads[i], NULL);
    }
    for (int i = 0; i < 10; i++)
    {
        pthread_join(pathStitcherThreads[i], NULL);
    }

    printGraph(graph);

    return 0;
}
