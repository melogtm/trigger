#ifndef PIPELINE_H
#define PIPELINE_H

#define FILE_MODE 0644

typedef enum { OP_NONE, OP_PIPE, OP_REDIR_IN, OP_REDIR_OUT, OP_REDIR_APPEND } Operator;

typedef struct {
    char **argv;
    char *infile;
    char *outfile;
    int append;
} PipelineStage;

typedef struct {
    int last_status;
} PipelineResult;

Operator classify_operator(const char *token);

PipelineStage *trigger_parse_pipeline(char **tokens, const int *glob_eligible, int *num_stages);
PipelineResult trigger_execute_pipeline(PipelineStage *stages, int num_stages);
void trigger_free_pipeline(PipelineStage *stages, int num_stages);

#endif
