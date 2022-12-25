#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 32768
#define COMMAND_NOT_FOUND 127

void show_prompt();
void read_command(char *input_buffer);
int parse_command(char *input_buffer, char **command_buffer);
void check_exit(char **command_buffer);

int main() {
  // 사용자 입력 버퍼
  char input_buffer[BUFFER_SIZE] = {'\0'};
  // 명령어 토큰 포인터 배열
  char *command_buffer[BUFFER_SIZE] = {NULL};

  // 무한히 반복하여 명령어를 입력 받는다
  while (1) {
    // 명령어를 입력 받는 대기 상태의 프롬프트를 출력한다
    show_prompt();
    // 명령어를 입력 받는다
    read_command(input_buffer);

    // 명령어를 파싱하여 명령행 인자로 전달가능한 형식으로 변환한다
    // parse_command의 반환값은 명령어 토큰의 개수이다
    // 명령어 토큰이 없는 경우 사용자에게 처음부터 다시 입력받는다
    if (parse_command(input_buffer, command_buffer) == 0) {
      continue;
    }

    // exit 명령어가 입력되었는지 체크한다
    check_exit(command_buffer);

    // fork 시스템콜로 my shell을 복제하여 자식 프로세스를 생성한다
    // fork는 부모 프로세스에게는 자식의 PID, 자식 프로세스에게는 0을 반환한다
    switch (fork()) {
      case 0:
        // 자식 프로세스인 경우
        // command를 실행한다
        // exec 시스템콜로 자식 프로세스를 명령어에 의한 프로세스로 덮어씌운다
        execvp(command_buffer[0], command_buffer);
        // exec 시스템콜이 성공했으면 프로세스 내용이 교체된다
        // 만약 성공했다면 이 라인은 실행되지 않는다
        // 이 라인에 진입했다면 exec에 실패한 것이니 에러 메시지를 출력한다
        printf("[error] failed to exec\n");
        // exec에 실패하면 my shell 프로세스인 상태로 남으니 종료시켜야 한다
        // "command not found" 라는 의미로 예약된 종료 코드인 127을 반환한다
        exit(COMMAND_NOT_FOUND);
        break;
      case -1:
        // fork 시스템콜이 실패한 경우
        // 에러 메시지를 출력한다
        printf("[error] failed to fork\n");
        break;
      default:
        // 부모 프로세스(my shell)인 경우
        // 자식 프로세스(command)가 종료되기까지 기다린다
        wait(NULL);
    }
  }

  return 0;
}

void show_prompt() {
  printf("najongwoo$ ");
}

void read_command(char *input_buffer) {
  // fgets로 버퍼 최대 크기까지만 문자열을 입력 받는다
  fgets(input_buffer, BUFFER_SIZE, stdin);
  // 입력 받은 문자열 끝의 개행 문자를 제거한다
  input_buffer[strcspn(input_buffer, "\r\n")] = '\0';
}

int parse_command(char *input_buffer, char **command_buffer) {
  // 명령어 토큰 개수
  int count = 0;
  // 명령어 토큰 포인터
  // 공백 문자로 토큰을 구분한다
  char *token = strtok(input_buffer, " \t");

  // 명령어 토큰을 끝까지 추출한다
  while (token != NULL) {
    // 명령어 토큰 포인터를 차례대로 배열에 저장하고 명령어 토큰 개수를 증가시킨다
    command_buffer[count++] = token;
    // 다음 토큰을 추출한다
    token = strtok(NULL, " \t");
  }

  // 명령어 토큰 포인터 배열에 마지막으로 널 포인터를 추가한다
  // exec 시스템콜 호출 시 명령행 인자의 끝 부분임을 알려준다
  command_buffer[count] = NULL;

  // 명령어 토큰 개수를 반환한다
  return count;
}

void check_exit(char **command_buffer) {
  // 명령어가 "exit"인 경우 my shell을 종료한다
  if (command_buffer[0]
      && command_buffer[1] == NULL
      && strcmp(command_buffer[0], "exit") == 0) {
    exit(0);
  }
}
