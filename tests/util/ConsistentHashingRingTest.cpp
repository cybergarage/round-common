/******************************************************************
 *
 * Round for C
 *
 * Copyright (C) Satoshi Konno 2015
 *
 * This is licensed under BSD-style license, see file COPYING.
 *
 ******************************************************************/

#include <boost/test/unit_test.hpp>
#include <round/util/consistent_hashing.h>

typedef struct {
  ROUND_CONSISTENTHASHING_NODE_STRUCT_MEMBERS
  char hashCode[2];
} RoundTestConsistentHashingNode;

const char* round_test_consistenthashing_node_gethash(
    RoundTestConsistentHashingNode* node)
{
  return node->hashCode;
}

RoundConsistentHashingNode* round_test_consistenthashing_ring_gethandlenode(
    RoundConsistentHashingRing* ring, int value)
{
  char hashCode[2];
  sprintf(hashCode, "%d", value);
  return round_consistenthashing_ring_gethandlenode(ring, hashCode);
}

RoundTestConsistentHashingNode*
round_test_consistenthashing_node_new(int value)
{
  RoundTestConsistentHashingNode* node;

  node = (RoundTestConsistentHashingNode*)malloc(
      sizeof(RoundTestConsistentHashingNode));

  if (!node)
    return NULL;

  round_consistenthashing_node_init((RoundConsistentHashingNode*)node);
  round_consistenthashing_node_sethashfunc(
      node, round_test_consistenthashing_node_gethash);
  sprintf(node->hashCode, "%d", value);

  return node;
}

bool round_test_consistenthashing_node_delete(
    RoundTestConsistentHashingNode* node)
{
  if (!node)
    return false;

  free(node);

  return true;
}

BOOST_AUTO_TEST_SUITE(conshash)

BOOST_AUTO_TEST_CASE(ConsistentHashingRingNew)
{
  RoundConsistentHashingRing* ring = round_consistenthashing_ring_new();
  BOOST_CHECK(ring);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_size(ring), 0);
  BOOST_CHECK(round_consistenthashing_ring_delete(ring));
}

BOOST_AUTO_TEST_CASE(ConsistentHashGraphAddSameNodeTest)
{
  RoundConsistentHashingRing* ring = round_consistenthashing_ring_new();
  round_consistenthashing_ring_setnodedestructor(
      ring, round_test_consistenthashing_node_delete);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_size(ring), 0);

  RoundTestConsistentHashingNode* node = round_test_consistenthashing_node_new(0);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_hasnode(ring, node), false);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getnodeindex(ring, node), -1);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_size(ring), 0);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_addnode(ring, node), true);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_hasnode(ring, node), true);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_size(ring), 1);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_islastnode(ring, node), true);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getequalnode(ring, node),
      (RoundConsistentHashingNode*)node);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getlastnode(ring),
      (RoundConsistentHashingNode*)node);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_addnode(ring, node), false);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_hasnode(ring, node), true);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_size(ring), 1);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_islastnode(ring, node), true);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getlastnode(ring),
      (RoundConsistentHashingNode*)node);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getequalnode(ring, node),
      (RoundConsistentHashingNode*)node);

  BOOST_CHECK(round_consistenthashing_ring_delete(ring));
}

BOOST_AUTO_TEST_CASE(ConsistentHashGraphAddTest)
{
  const int conNodeCount = 9;

  RoundConsistentHashingRing* ring = round_consistenthashing_ring_new();
  round_consistenthashing_ring_setnodedestructor(
      ring, round_test_consistenthashing_node_delete);

  RoundTestConsistentHashingNode* nodes[conNodeCount];

  // Added new nodes

  for (int n = 0; n < conNodeCount; n++) {
    nodes[n] = round_test_consistenthashing_node_new(n + 1);
  }
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_size(ring), 0);
  for (int n = 0; n < conNodeCount; n++) {
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_hasnode(ring, nodes[n]),
        false);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getnodeindex(ring, nodes[n]),
        -1);

    BOOST_CHECK_EQUAL(round_consistenthashing_ring_addnode(ring, nodes[n]),
        true);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_hasnode(ring, nodes[n]),
        true);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_size(ring), (n + 1));
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getnodeindex(ring, nodes[n]),
        n);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getequalnode(ring, nodes[n]),
        (RoundConsistentHashingNode*)nodes[n]);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_islastnode(ring, nodes[n]),
        true);
  }

  // Clear nodes

  round_consistenthashing_ring_clear(ring);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_size(ring), 0);

  // Added same nodes again

  for (int n = 0; n < conNodeCount; n++) {
    nodes[n] = round_test_consistenthashing_node_new(n + 1);
  }
  for (int n = (conNodeCount - 1); 0 <= n; n--) {
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getnodeindex(ring, nodes[n]),
        -1);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_addnode(ring, nodes[n]),
        true);
    BOOST_CHECK_EQUAL(
        round_consistenthashing_ring_islastnode(ring, nodes[conNodeCount - 1]),
        true);
  }
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_size(ring), conNodeCount);
  for (int n = 0; n < conNodeCount; n++) {
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getnodeindex(ring, nodes[n]),
        n);
  }

  round_consistenthashing_ring_clear(ring);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_size(ring), 0);

  BOOST_CHECK(round_consistenthashing_ring_delete(ring));
}

BOOST_AUTO_TEST_CASE(ConsistentHashGraphRemoveTest)
{
  const int conNodeCount = 9;

  RoundConsistentHashingRing* ring = round_consistenthashing_ring_new();
  round_consistenthashing_ring_setnodedestructor(
      ring, round_test_consistenthashing_node_delete);

  RoundTestConsistentHashingNode* nodes[conNodeCount];

  for (int n = 0; n < conNodeCount; n++) {
    nodes[n] = round_test_consistenthashing_node_new(n + 1);
  }
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_size(ring), 0);

  for (int n = 0; n < conNodeCount; n++) {
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_hasnode(ring, nodes[n]),
        false);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getnodeindex(ring, nodes[n]),
        -1);

    BOOST_CHECK_EQUAL(round_consistenthashing_ring_addnode(ring, nodes[n]),
        true);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_hasnode(ring, nodes[n]),
        true);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_size(ring), (n + 1));
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getnodeindex(ring, nodes[n]),
        n);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getequalnode(ring, nodes[n]),
        (RoundConsistentHashingNode*)nodes[n]);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_islastnode(ring, nodes[n]),
        true);
  }

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_size(ring), conNodeCount);

  for (int n = 0; n < conNodeCount; n++) {
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_hasnode(ring, nodes[n]),
        true);

    BOOST_CHECK_EQUAL(round_consistenthashing_ring_removenode(ring, nodes[n]),
        true);

    BOOST_CHECK_EQUAL(round_consistenthashing_ring_hasnode(ring, nodes[n]),
        false);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getnodeindex(ring, nodes[n]),
        -1);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getequalnode(ring, nodes[n]),
        (RoundConsistentHashingNode*)NULL);
  }

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_size(ring), 0);

  BOOST_CHECK(round_consistenthashing_ring_delete(ring));
}

BOOST_AUTO_TEST_CASE(ConsistentHashGraphEqualsTest)
{
  const int conNodeCount = 9;
  RoundConsistentHashingRing* ring = round_consistenthashing_ring_new();
  RoundTestConsistentHashingNode* nodes[conNodeCount];

  for (int n = 0; n < conNodeCount; n++) {
    nodes[n] = round_test_consistenthashing_node_new(n + 1);
    BOOST_CHECK(round_consistenthashing_ring_addnode(ring, nodes[n]));
  }

  for (int i = 0; i < conNodeCount; i++) {
    for (int j = 0; j < conNodeCount; j++) {
      BOOST_CHECK_EQUAL(round_consistenthashing_node_equals(nodes[i], nodes[j]),
          ((i == j) ? true : false));
    }
  }

  BOOST_CHECK(round_consistenthashing_ring_delete(ring));
}

BOOST_AUTO_TEST_CASE(ConsistentHashGraphCompareTest)
{
  const int conNodeCount = 4;
  RoundTestConsistentHashingNode* nodes[conNodeCount];

  RoundTestConsistentHashingNode* node2 = nodes[0] = round_test_consistenthashing_node_new(2);
  RoundTestConsistentHashingNode* node4 = nodes[1] = round_test_consistenthashing_node_new(4);
  RoundTestConsistentHashingNode* node6 = nodes[2] = round_test_consistenthashing_node_new(6);
  RoundTestConsistentHashingNode* node8 = nodes[3] = round_test_consistenthashing_node_new(8);

  BOOST_CHECK_EQUAL(round_consistenthashing_node_comp(node2, node2),
      RoundListNodeCompareSame);
  BOOST_CHECK_EQUAL(round_consistenthashing_node_comp(node2, node4),
      RoundListNodeCompareGreater);
  BOOST_CHECK_EQUAL(round_consistenthashing_node_comp(node2, node6),
      RoundListNodeCompareGreater);
  BOOST_CHECK_EQUAL(round_consistenthashing_node_comp(node2, node8),
      RoundListNodeCompareGreater);

  BOOST_CHECK_EQUAL(round_consistenthashing_node_comp(node4, node2),
      RoundListNodeCompareLess);
  BOOST_CHECK_EQUAL(round_consistenthashing_node_comp(node4, node4),
      RoundListNodeCompareSame);
  BOOST_CHECK_EQUAL(round_consistenthashing_node_comp(node4, node6),
      RoundListNodeCompareGreater);
  BOOST_CHECK_EQUAL(round_consistenthashing_node_comp(node4, node8),
      RoundListNodeCompareGreater);

  BOOST_CHECK_EQUAL(round_consistenthashing_node_comp(node6, node2),
      RoundListNodeCompareLess);
  BOOST_CHECK_EQUAL(round_consistenthashing_node_comp(node6, node4),
      RoundListNodeCompareLess);
  BOOST_CHECK_EQUAL(round_consistenthashing_node_comp(node6, node6),
      RoundListNodeCompareSame);
  BOOST_CHECK_EQUAL(round_consistenthashing_node_comp(node6, node8),
      RoundListNodeCompareGreater);

  BOOST_CHECK_EQUAL(round_consistenthashing_node_comp(node8, node2),
      RoundListNodeCompareLess);
  BOOST_CHECK_EQUAL(round_consistenthashing_node_comp(node8, node4),
      RoundListNodeCompareLess);
  BOOST_CHECK_EQUAL(round_consistenthashing_node_comp(node8, node6),
      RoundListNodeCompareLess);
  BOOST_CHECK_EQUAL(round_consistenthashing_node_comp(node8, node8),
      RoundListNodeCompareSame);

  for (int n = 0; n < conNodeCount; n++) {
    round_test_consistenthashing_node_delete(nodes[n]);
  }
}

BOOST_AUTO_TEST_CASE(ConsistentHashGraphHandleTest)
{
  const int conNodeCount = 4;
  RoundConsistentHashingRing* ring = round_consistenthashing_ring_new();
  RoundTestConsistentHashingNode* nodes[conNodeCount];

  RoundTestConsistentHashingNode* node2 = nodes[0] = round_test_consistenthashing_node_new(2);
  RoundTestConsistentHashingNode* node4 = nodes[1] = round_test_consistenthashing_node_new(4);
  RoundTestConsistentHashingNode* node6 = nodes[2] = round_test_consistenthashing_node_new(6);
  RoundTestConsistentHashingNode* node8 = nodes[3] = round_test_consistenthashing_node_new(8);

  for (int n = 0; n < conNodeCount; n++) {
    round_consistenthashing_ring_addnode(ring, nodes[n]);
  }

  // 2-4-6-8
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 0)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 1)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 2)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 3)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 4)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 5)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 6)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 7)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 8)),
      round_consistenthashing_ring_getnodeindex(ring, node8));

  // 4-6-8
  round_consistenthashing_node_remove(node2);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 0)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 1)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 2)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 3)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 4)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 5)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 6)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 7)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 8)),
      round_consistenthashing_ring_getnodeindex(ring, node8));

  // 2-4-6-8
  round_consistenthashing_ring_addnode(ring, node2);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 0)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 1)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 2)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 3)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 4)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 5)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 6)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 7)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 8)),
      round_consistenthashing_ring_getnodeindex(ring, node8));

  // 2-6-8
  round_consistenthashing_node_remove(node4);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 0)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 1)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 2)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 3)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 4)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 5)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 6)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 7)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 8)),
      round_consistenthashing_ring_getnodeindex(ring, node8));

  // 2-4-6-8
  round_consistenthashing_ring_addnode(ring, node4);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 0)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 1)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 2)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 3)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 4)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 5)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 6)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 7)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 8)),
      round_consistenthashing_ring_getnodeindex(ring, node8));

  // 2-4-8
  round_consistenthashing_node_remove(node6);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 0)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 1)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 2)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 3)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 4)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 5)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 6)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 7)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 8)),
      round_consistenthashing_ring_getnodeindex(ring, node8));

  // 2-4-6-8
  round_consistenthashing_ring_addnode(ring, node6);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 0)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 1)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 2)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 3)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 4)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 5)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 6)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 7)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 8)),
      round_consistenthashing_ring_getnodeindex(ring, node8));

  // 2-4-6
  round_consistenthashing_node_remove(node8);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 0)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 1)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 2)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 3)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 4)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(ring, round_test_consistenthashing_ring_gethandlenode(ring, 5)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 6)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 7)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getnodeindex(ring, round_test_consistenthashing_ring_gethandlenode(ring, 8)), round_consistenthashing_ring_getnodeindex(ring, node6));

  // 2-4-6-8
  round_consistenthashing_ring_addnode(ring, node8);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 0)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 1)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 2)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 3)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 4)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 5)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 6)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 7)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 8)),
      round_consistenthashing_ring_getnodeindex(ring, node8));

  // 2-6
  round_consistenthashing_node_remove(node4);
  round_consistenthashing_node_remove(node8);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 0)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 1)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 2)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 3)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 4)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 5)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 6)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 7)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 8)),
      round_consistenthashing_ring_getnodeindex(ring, node6));

  // 2-4-6-8
  round_consistenthashing_ring_addnode(ring, node4);
  round_consistenthashing_ring_addnode(ring, node8);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 0)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 1)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 2)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 3)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 4)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 5)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 6)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 7)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 8)),
      round_consistenthashing_ring_getnodeindex(ring, node8));

  // 4-8
  round_consistenthashing_node_remove(node2);
  round_consistenthashing_node_remove(node6);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 0)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 1)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 2)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 3)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 4)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 5)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 6)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 7)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 8)),
      round_consistenthashing_ring_getnodeindex(ring, node8));

  // 2-4-6-8
  round_consistenthashing_ring_addnode(ring, node2);
  round_consistenthashing_ring_addnode(ring, node6);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 0)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 1)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 2)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 3)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 4)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 5)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 6)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 7)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 8)),
      round_consistenthashing_ring_getnodeindex(ring, node8));

  // 2
  round_consistenthashing_node_remove(node4);
  round_consistenthashing_node_remove(node6);
  round_consistenthashing_node_remove(node8);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 0)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 1)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 2)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 3)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 4)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 5)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 6)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 7)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 8)),
      round_consistenthashing_ring_getnodeindex(ring, node2));

  // 2-4-6-8
  round_consistenthashing_ring_addnode(ring, node4);
  round_consistenthashing_ring_addnode(ring, node6);
  round_consistenthashing_ring_addnode(ring, node8);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 0)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 1)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 2)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 3)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 4)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 5)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 6)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 7)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 8)),
      round_consistenthashing_ring_getnodeindex(ring, node8));

  // 8
  round_consistenthashing_node_remove(node2);
  round_consistenthashing_node_remove(node4);
  round_consistenthashing_node_remove(node6);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 0)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 1)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 2)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 3)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 4)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 5)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 6)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 7)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 8)),
      round_consistenthashing_ring_getnodeindex(ring, node8));

  // 2-4-6-8
  round_consistenthashing_ring_addnode(ring, node2);
  round_consistenthashing_ring_addnode(ring, node4);
  round_consistenthashing_ring_addnode(ring, node6);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 0)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 1)),
      round_consistenthashing_ring_getnodeindex(ring, node8));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 2)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 3)),
      round_consistenthashing_ring_getnodeindex(ring, node2));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 4)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 5)),
      round_consistenthashing_ring_getnodeindex(ring, node4));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 6)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 7)),
      round_consistenthashing_ring_getnodeindex(ring, node6));
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getnodeindex(
          ring, round_test_consistenthashing_ring_gethandlenode(ring, 8)),
      round_consistenthashing_ring_getnodeindex(ring, node8));

  BOOST_CHECK(round_consistenthashing_ring_delete(ring));
}

BOOST_AUTO_TEST_CASE(ConsistentHashGraphOffsetNodeTest)
{
  const int conNodeCount = 9;
  RoundConsistentHashingRing* ring = round_consistenthashing_ring_new();
  RoundConsistentHashingNode* nodes[conNodeCount];

  for (int n = 0; n < conNodeCount; n++) {
    nodes[n] = (RoundConsistentHashingNode*)round_test_consistenthashing_node_new(n + 1);
    round_consistenthashing_ring_addnode(ring, nodes[n]);
  }

  for (int baseOffset = 0; baseOffset <= conNodeCount;
       baseOffset += conNodeCount) {
    // +1
    for (int n = 0; n < (conNodeCount - 1); n++) {
      BOOST_CHECK_EQUAL(round_consistenthashing_node_next(nodes[n]),
          nodes[n + 1]);
      BOOST_CHECK_EQUAL(round_consistenthashing_ring_getoffsetnode(
                            ring, nodes[n], (+1 + baseOffset)),
          nodes[n + 1]);
    }
    BOOST_CHECK_EQUAL(
        round_consistenthashing_node_nextcircular(nodes[conNodeCount - 1]),
        nodes[0]);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getoffsetnode(
                          ring, nodes[conNodeCount - 1], (+1 + baseOffset)),
        nodes[0]);

    // (+2 +  baseOffset)
    for (int n = 0; n < (conNodeCount - 2); n++) {
      BOOST_CHECK_EQUAL(round_consistenthashing_ring_getoffsetnode(
                            ring, nodes[n], (+2 + baseOffset)),
          nodes[n + 2]);
    }
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getoffsetnode(
                          ring, nodes[conNodeCount - 2], (+2 + baseOffset)),
        nodes[0]);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getoffsetnode(
                          ring, nodes[conNodeCount - 1], (+2 + baseOffset)),
        nodes[1]);

    // (+3+  baseOffset)
    for (int n = 0; n < (conNodeCount - 3); n++) {
      BOOST_CHECK_EQUAL(round_consistenthashing_ring_getoffsetnode(
                            ring, nodes[n], (+3 + baseOffset)),
          nodes[n + 3]);
    }
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getoffsetnode(
                          ring, nodes[conNodeCount - 3], (+3 + baseOffset)),
        nodes[0]);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getoffsetnode(
                          ring, nodes[conNodeCount - 2], (+3 + baseOffset)),
        nodes[1]);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getoffsetnode(
                          ring, nodes[conNodeCount - 1], (+3 + baseOffset)),
        nodes[2]);

    // (-1 -  baseOffset)
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getprevnode(ring, nodes[0]),
        nodes[conNodeCount - 1]);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getoffsetnode(
                          ring, nodes[0], (-1 - baseOffset)),
        nodes[conNodeCount - 1]);
    for (int n = 1; n < conNodeCount; n++) {
      BOOST_CHECK_EQUAL(
          round_consistenthashing_ring_getprevnode(ring, nodes[n]),
          nodes[n - 1]);
      BOOST_CHECK_EQUAL(round_consistenthashing_ring_getoffsetnode(
                            ring, nodes[n], (-1 - baseOffset)),
          nodes[n - 1]);
    }

    // (-2 -  baseOffset)
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getoffsetnode(
                          ring, nodes[0], (-2 - baseOffset)),
        nodes[conNodeCount - 2]);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getoffsetnode(
                          ring, nodes[1], (-2 - baseOffset)),
        nodes[conNodeCount - 1]);
    for (int n = 2; n < conNodeCount; n++) {
      BOOST_CHECK_EQUAL(round_consistenthashing_ring_getoffsetnode(
                            ring, nodes[n], (-2 - baseOffset)),
          nodes[n - 2]);
    }

    // (-3 -  baseOffset)
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getoffsetnode(
                          ring, nodes[0], (-3 - baseOffset)),
        nodes[conNodeCount - 3]);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getoffsetnode(
                          ring, nodes[1], (-3 - baseOffset)),
        nodes[conNodeCount - 2]);
    BOOST_CHECK_EQUAL(round_consistenthashing_ring_getoffsetnode(
                          ring, nodes[2], (-3 - baseOffset)),
        nodes[conNodeCount - 1]);
    for (int n = 3; n < conNodeCount; n++) {
      BOOST_CHECK_EQUAL(round_consistenthashing_ring_getoffsetnode(
                            ring, nodes[n], (-3 - baseOffset)),
          nodes[n - 3]);
    }
  }

  BOOST_CHECK(round_consistenthashing_ring_delete(ring));
}

BOOST_AUTO_TEST_CASE(ConsistentHashGraphFowardDistanceTest)
{
  const int conNodeCount = 9;
  RoundConsistentHashingRing* ring = round_consistenthashing_ring_new();
  RoundTestConsistentHashingNode* nodes[conNodeCount];

  for (int n = 0; n < conNodeCount; n++) {
    nodes[n] = round_test_consistenthashing_node_new(n + 1);
    round_consistenthashing_ring_addnode(ring, nodes[n]);
  }

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[0], nodes[0]),
      0);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[0], nodes[1]),
      1);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[0], nodes[2]),
      2);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[0], nodes[3]),
      3);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[0], nodes[4]),
      4);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[0], nodes[5]),
      5);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[0], nodes[6]),
      6);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[0], nodes[7]),
      7);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[0], nodes[8]),
      8);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[1], nodes[0]),
      8);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[1], nodes[1]),
      0);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[1], nodes[2]),
      1);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[1], nodes[3]),
      2);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[1], nodes[4]),
      3);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[1], nodes[5]),
      4);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[1], nodes[6]),
      5);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[1], nodes[7]),
      6);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[1], nodes[8]),
      7);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[2], nodes[0]),
      7);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[2], nodes[1]),
      8);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[2], nodes[2]),
      0);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[2], nodes[3]),
      1);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[2], nodes[4]),
      2);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[2], nodes[5]),
      3);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[2], nodes[6]),
      4);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[2], nodes[7]),
      5);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[2], nodes[8]),
      6);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[3], nodes[0]),
      6);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[3], nodes[1]),
      7);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[3], nodes[2]),
      8);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[3], nodes[3]),
      0);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[3], nodes[4]),
      1);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[3], nodes[5]),
      2);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[3], nodes[6]),
      3);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[3], nodes[7]),
      4);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[3], nodes[8]),
      5);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[4], nodes[0]),
      5);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[4], nodes[1]),
      6);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[4], nodes[2]),
      7);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[4], nodes[3]),
      8);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[4], nodes[4]),
      0);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[4], nodes[5]),
      1);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[4], nodes[6]),
      2);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[4], nodes[7]),
      3);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[4], nodes[8]),
      4);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[5], nodes[0]),
      4);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[5], nodes[1]),
      5);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[5], nodes[2]),
      6);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[5], nodes[3]),
      7);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[5], nodes[4]),
      8);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[5], nodes[5]),
      0);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[5], nodes[6]),
      1);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[5], nodes[7]),
      2);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[5], nodes[8]),
      3);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[6], nodes[0]),
      3);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[6], nodes[1]),
      4);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[6], nodes[2]),
      5);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[6], nodes[3]),
      6);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[6], nodes[4]),
      7);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[6], nodes[5]),
      8);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[6], nodes[6]),
      0);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[6], nodes[7]),
      1);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[6], nodes[8]),
      2);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[7], nodes[0]),
      2);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[7], nodes[1]),
      3);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[7], nodes[2]),
      4);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[7], nodes[3]),
      5);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[7], nodes[4]),
      6);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[7], nodes[5]),
      7);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[7], nodes[6]),
      8);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[7], nodes[7]),
      0);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[7], nodes[8]),
      1);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[8], nodes[0]),
      1);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[8], nodes[1]),
      2);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[8], nodes[2]),
      3);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[8], nodes[3]),
      4);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[8], nodes[4]),
      5);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[8], nodes[5]),
      6);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[8], nodes[6]),
      7);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[8], nodes[7]),
      8);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getforwardnodedistance(
                        ring, nodes[8], nodes[8]),
      0);

  BOOST_CHECK(round_consistenthashing_ring_delete(ring));
}

BOOST_AUTO_TEST_CASE(ConsistentHashGraphBackfowardDistanceTest)
{
  const int conNodeCount = 9;
  RoundConsistentHashingRing* ring = round_consistenthashing_ring_new();
  RoundTestConsistentHashingNode* nodes[conNodeCount];

  for (int n = 0; n < conNodeCount; n++) {
    nodes[n] = round_test_consistenthashing_node_new(n + 1);
    round_consistenthashing_ring_addnode(ring, nodes[n]);
  }

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[0], nodes[0]),
      0);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[0], nodes[1]),
      -8);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[0], nodes[2]),
      -7);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[0], nodes[3]),
      -6);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[0], nodes[4]),
      -5);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[0], nodes[5]),
      -4);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[0], nodes[6]),
      -3);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[0], nodes[7]),
      -2);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[0], nodes[8]),
      -1);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[1], nodes[0]),
      -1);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[1], nodes[1]),
      0);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[1], nodes[2]),
      -8);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[1], nodes[3]),
      -7);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[1], nodes[4]),
      -6);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[1], nodes[5]),
      -5);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[1], nodes[6]),
      -4);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[1], nodes[7]),
      -3);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[1], nodes[8]),
      -2);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[2], nodes[0]),
      -2);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[2], nodes[1]),
      -1);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[2], nodes[2]),
      0);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[2], nodes[3]),
      -8);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[2], nodes[4]),
      -7);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[2], nodes[5]),
      -6);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[2], nodes[6]),
      -5);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[2], nodes[7]),
      -4);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[2], nodes[8]),
      -3);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[3], nodes[0]),
      -3);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[3], nodes[1]),
      -2);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[3], nodes[2]),
      -1);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[3], nodes[3]),
      0);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[3], nodes[4]),
      -8);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[3], nodes[5]),
      -7);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[3], nodes[6]),
      -6);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[3], nodes[7]),
      -5);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[3], nodes[8]),
      -4);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[4], nodes[0]),
      -4);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[4], nodes[1]),
      -3);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[4], nodes[2]),
      -2);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[4], nodes[3]),
      -1);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[4], nodes[4]),
      0);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[4], nodes[5]),
      -8);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[4], nodes[6]),
      -7);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[4], nodes[7]),
      -6);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[4], nodes[8]),
      -5);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[5], nodes[0]),
      -5);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[5], nodes[1]),
      -4);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[5], nodes[2]),
      -3);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[5], nodes[3]),
      -2);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[5], nodes[4]),
      -1);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[5], nodes[5]),
      0);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[5], nodes[6]),
      -8);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[5], nodes[7]),
      -7);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[5], nodes[8]),
      -6);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[6], nodes[0]),
      -6);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[6], nodes[1]),
      -5);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[6], nodes[2]),
      -4);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[6], nodes[3]),
      -3);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[6], nodes[4]),
      -2);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[6], nodes[5]),
      -1);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[6], nodes[6]),
      0);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[6], nodes[7]),
      -8);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[6], nodes[8]),
      -7);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[7], nodes[0]),
      -7);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[7], nodes[1]),
      -6);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[7], nodes[2]),
      -5);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[7], nodes[3]),
      -4);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[7], nodes[4]),
      -3);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[7], nodes[5]),
      -2);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[7], nodes[6]),
      -1);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[7], nodes[7]),
      0);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[7], nodes[8]),
      -8);

  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[8], nodes[0]),
      -8);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[8], nodes[1]),
      -7);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[8], nodes[2]),
      -6);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[8], nodes[3]),
      -5);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[8], nodes[4]),
      -4);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[8], nodes[5]),
      -3);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[8], nodes[6]),
      -2);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[8], nodes[7]),
      -1);
  BOOST_CHECK_EQUAL(round_consistenthashing_ring_getbackwardnodedistance(
                        ring, nodes[8], nodes[8]),
      0);

  BOOST_CHECK(round_consistenthashing_ring_delete(ring));
}

BOOST_AUTO_TEST_CASE(ConsistentHashGraphMinDistanceTest)
{
  const int conNodeCount = 9;
  RoundConsistentHashingRing* ring = round_consistenthashing_ring_new();
  RoundTestConsistentHashingNode* nodes[conNodeCount];

  for (int n = 0; n < conNodeCount; n++) {
    nodes[n] = round_test_consistenthashing_node_new(n + 1);
    round_consistenthashing_ring_addnode(ring, nodes[n]);
  }

  for (int i = 0; i < conNodeCount; i++) {
    for (int j = 0; j < conNodeCount; j++) {
      off_t distance = round_consistenthashing_ring_getminnodedistance(
          ring, nodes[i], nodes[j]);
      BOOST_CHECK(distance <= (conNodeCount / 2));
    }
  }

  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[0], nodes[0]),
      0);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[0], nodes[1]),
      1);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[0], nodes[2]),
      2);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[0], nodes[3]),
      3);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[0], nodes[4]),
      4);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[0], nodes[5]),
      -4);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[0], nodes[6]),
      -3);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[0], nodes[7]),
      -2);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[0], nodes[8]),
      -1);

  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[1], nodes[0]),
      -1);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[1], nodes[1]),
      0);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[1], nodes[2]),
      1);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[1], nodes[3]),
      2);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[1], nodes[4]),
      3);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[1], nodes[5]),
      4);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[1], nodes[6]),
      -4);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[1], nodes[7]),
      -3);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[1], nodes[8]),
      -2);

  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[2], nodes[0]),
      -2);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[2], nodes[1]),
      -1);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[2], nodes[2]),
      0);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[2], nodes[3]),
      1);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[2], nodes[4]),
      2);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[2], nodes[5]),
      3);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[2], nodes[6]),
      4);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[2], nodes[7]),
      -4);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[2], nodes[8]),
      -3);

  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[3], nodes[0]),
      -3);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[3], nodes[1]),
      -2);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[3], nodes[2]),
      -1);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[3], nodes[3]),
      0);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[3], nodes[4]),
      1);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[3], nodes[5]),
      2);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[3], nodes[6]),
      3);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[3], nodes[7]),
      4);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[3], nodes[8]),
      -4);

  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[4], nodes[0]),
      -4);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[4], nodes[1]),
      -3);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[4], nodes[2]),
      -2);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[4], nodes[3]),
      -1);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[4], nodes[4]),
      0);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[4], nodes[5]),
      1);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[4], nodes[6]),
      2);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[4], nodes[7]),
      3);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[4], nodes[8]),
      4);

  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[5], nodes[0]),
      4);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[5], nodes[1]),
      -4);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[5], nodes[2]),
      -3);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[5], nodes[3]),
      -2);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[5], nodes[4]),
      -1);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[5], nodes[5]),
      0);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[5], nodes[6]),
      1);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[5], nodes[7]),
      2);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[5], nodes[8]),
      3);

  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[6], nodes[0]),
      3);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[6], nodes[1]),
      4);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[6], nodes[2]),
      -4);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[6], nodes[3]),
      -3);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[6], nodes[4]),
      -2);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[6], nodes[5]),
      -1);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[6], nodes[6]),
      0);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[6], nodes[7]),
      1);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[6], nodes[8]),
      2);

  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[7], nodes[0]),
      2);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[7], nodes[1]),
      3);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[7], nodes[2]),
      4);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[7], nodes[3]),
      -4);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[7], nodes[4]),
      -3);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[7], nodes[5]),
      -2);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[7], nodes[6]),
      -1);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[7], nodes[7]),
      0);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[7], nodes[8]),
      1);

  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[8], nodes[0]),
      1);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[8], nodes[1]),
      2);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[8], nodes[2]),
      3);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[8], nodes[3]),
      4);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[8], nodes[4]),
      -4);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[8], nodes[5]),
      -3);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[8], nodes[6]),
      -2);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[8], nodes[7]),
      -1);
  BOOST_CHECK_EQUAL(
      round_consistenthashing_ring_getminnodedistance(ring, nodes[8], nodes[8]),
      0);

  BOOST_CHECK(round_consistenthashing_ring_delete(ring));
}

BOOST_AUTO_TEST_CASE(ConsistentHashGraphIsHandleNodeTest)
{
  const int conNodeCount = 9;
  RoundConsistentHashingRing* ring = round_consistenthashing_ring_new();
  RoundTestConsistentHashingNode* nodes[conNodeCount];

  for (int n = 0; n < conNodeCount; n++) {
    nodes[n] = round_test_consistenthashing_node_new(n + 1);
    round_consistenthashing_ring_addnode(ring, nodes[n]);
  }

  for (int i = 0; i < conNodeCount; i++) {
    for (int j = 0; j < conNodeCount; j++) {
      const char* hashCode = round_consistenthashing_node_gethash(nodes[j]);
      BOOST_CHECK_EQUAL(
          round_consistenthashing_ring_ishandlenode(ring, nodes[i], hashCode),
          (i == j));
    }
  }

  BOOST_CHECK(round_consistenthashing_ring_delete(ring));
}

BOOST_AUTO_TEST_SUITE_END()
