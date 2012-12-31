/* build.h: Build header file */

#ifndef _BUILD_H
#define _BUILD_H

pre_disk_part_t
*part_node_create(void);

pre_disk_part_t
*part_node_add(pre_disk_part_t *head_node, MYSQL_ROW part_row);

int
part_node_delete(pre_disk_part_t head_node);

void
part_node_free(void);

int
check_for_special_partition(pre_disk_part_t *part_info);

void
chomp(char *input);

#endif /* _BUILD_H */