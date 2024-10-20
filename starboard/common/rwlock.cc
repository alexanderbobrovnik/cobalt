// Copyright 2017 The Cobalt Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "starboard/common/rwlock.h"

#include "starboard/common/log.h"

namespace starboard {

// Write-preferring lock.
// https://en.wikipedia.org/wiki/Readers%E2%80%93writer_lock
RWLock::RWLock() : readers_(0), writing_(false) {
  SB_CHECK(pthread_mutex_init(&mutex_, nullptr) == 0);
  SB_CHECK(pthread_cond_init(&condition_, nullptr) == 0);
}

RWLock::~RWLock() {
  pthread_cond_destroy(&condition_);
  pthread_mutex_destroy(&mutex_);
}

void RWLock::AcquireReadLock() {
  SB_CHECK(pthread_mutex_lock(&mutex_) == 0);
  while (writing_) {
    SB_CHECK(pthread_cond_wait(&condition_, &mutex_) == 0);
  }
  ++readers_;
  SB_CHECK(pthread_mutex_unlock(&mutex_) == 0);
}

void RWLock::ReleaseReadLock() {
  SB_CHECK(pthread_mutex_lock(&mutex_) == 0);
  if (--readers_ == 0) {
    SB_CHECK(pthread_cond_broadcast(&condition_) == 0);
  }
  SB_CHECK(pthread_mutex_unlock(&mutex_) == 0);
}

void RWLock::AcquireWriteLock() {
  SB_CHECK(pthread_mutex_lock(&mutex_) == 0);
  while (writing_) {
    SB_CHECK(pthread_cond_wait(&condition_, &mutex_) == 0);
  }
  writing_ = true;
  while (readers_ > 0) {
    SB_CHECK(pthread_cond_wait(&condition_, &mutex_) == 0);
  }
  SB_CHECK(pthread_mutex_unlock(&mutex_) == 0);
}

void RWLock::ReleaseWriteLock() {
  SB_CHECK(pthread_mutex_lock(&mutex_) == 0);
  writing_ = false;
  SB_CHECK(pthread_cond_broadcast(&condition_) == 0);
  SB_CHECK(pthread_mutex_unlock(&mutex_) == 0);
}

}  // namespace starboard
