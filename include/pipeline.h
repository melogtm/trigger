#ifndef PIPELINE_H
#define PIPELINE_H

typedef struct {
    char **argv;
    char *infile;
    char *outfile;
    int append;
} PipelineStage;

PipelineStage *trigger_parse_pipeline(char **tokens, int *num_stages);
int trigger_execute_pipeline(PipelineStage *stages, int num_stages);

#endif
