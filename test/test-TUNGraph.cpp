#include <gtest/gtest.h>

#include "Snap.h"

// Test the default constructor
TEST(TUNGraph, DefaultConstructor) {
  PUNGraph Graph;

  Graph = TUNGraph::New();

  EXPECT_EQ(0,Graph->GetNodes());
  EXPECT_EQ(0,Graph->GetEdges());

  EXPECT_EQ(1,Graph->IsOk());
  EXPECT_EQ(1,Graph->Empty());
  EXPECT_EQ(0,Graph->HasFlag(gfDirected));
}

// Test node, edge creation
TEST(TUNGraph, ManipulateNodesEdges) {
  int NNodes = 10000;
  int NEdges = 100000;
  const char *FName = "test.graph.dat";

  PUNGraph Graph;
  PUNGraph Graph1;
  PUNGraph Graph2;
  int i;
  int n;
  int NCount;
  int LCount;
  int x,y;
  int Deg, InDeg, OutDeg;

  Graph = TUNGraph::New();
  EXPECT_EQ(1,Graph->Empty());

  // create the nodes
  for (i = 0; i < NNodes; i++) {
    Graph->AddNode(i);
  }
  EXPECT_EQ(0,Graph->Empty());
  EXPECT_EQ(NNodes,Graph->GetNodes());
  
  // create random edges
  NCount = NEdges;
  LCount = 0;
  while (NCount > 0) {
    x = (long) (drand48() * NNodes);
    y = (long) (drand48() * NNodes);
    if (!Graph->IsEdge(x,y)) {
      n = Graph->AddEdge(x, y);
      NCount--;
      if (x == y) {
        LCount++;
      }
    }
  }

  EXPECT_EQ(NEdges,Graph->GetEdges());

  EXPECT_EQ(0,Graph->Empty());
  EXPECT_EQ(1,Graph->IsOk());

  for (i = 0; i < NNodes; i++) {
    EXPECT_EQ(1,Graph->IsNode(i));
  }

  EXPECT_EQ(0,Graph->IsNode(NNodes));
  EXPECT_EQ(0,Graph->IsNode(NNodes+1));
  EXPECT_EQ(0,Graph->IsNode(2*NNodes));

  // nodes iterator
  NCount = 0;
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    NCount++;
  }
  EXPECT_EQ(NNodes,NCount);

  // edges per node iterator, each non-loop is visited twice, each loop once
  NCount = 0;
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    for (int e = 0; e < NI.GetOutDeg(); e++) {
      NCount++;
    }
  }
  EXPECT_EQ(NEdges*2-LCount,NCount);

  // edges iterator, each edge is visited once
  NCount = 0;
  for (TUNGraph::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
    NCount++;
  }
  EXPECT_EQ(NEdges,NCount);

  // node degree
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    Deg = NI.GetDeg();
    InDeg = NI.GetInDeg();
    OutDeg = NI.GetOutDeg();

    EXPECT_EQ(Deg,InDeg);
    EXPECT_EQ(Deg,OutDeg);
  }

  // assignment
  Graph1 = TUNGraph::New();
  *Graph1 = *Graph;

  EXPECT_EQ(NNodes,Graph1->GetNodes());
  EXPECT_EQ(NEdges,Graph1->GetEdges());
  EXPECT_EQ(0,Graph1->Empty());
  EXPECT_EQ(1,Graph1->IsOk());

  // saving and loading
  {
    TFOut FOut(FName);
    Graph->Save(FOut);
    FOut.Flush();
  }

  {
    TFIn FIn(FName);
    Graph2 = TUNGraph::Load(FIn);
  }

  EXPECT_EQ(NNodes,Graph2->GetNodes());
  EXPECT_EQ(NEdges,Graph2->GetEdges());
  EXPECT_EQ(0,Graph2->Empty());
  EXPECT_EQ(1,Graph2->IsOk());

  // remove all the nodes and edges
  for (i = 0; i < NNodes; i++) {
    n = Graph->GetRndNId();
    Graph->DelNode(n);
  }

  EXPECT_EQ(0,Graph->GetNodes());
  EXPECT_EQ(0,Graph->GetEdges());

  EXPECT_EQ(1,Graph->IsOk());
  EXPECT_EQ(1,Graph->Empty());

  Graph1->Clr();

  EXPECT_EQ(0,Graph1->GetNodes());
  EXPECT_EQ(0,Graph1->GetEdges());

  EXPECT_EQ(1,Graph1->IsOk());
  EXPECT_EQ(1,Graph1->Empty());
}

// Test edge iterator, manipulate edges
TEST(TUNGraph, ManipulateEdges) {
  int Iterations = 100;
  
  int NNodes;
  int NNodesStart = 8;
  int NNodesEnd = 25;
  
  int NEdges;
  int NEdgesStart = 0;
  int NEdgesEnd = 50;
  
  PUNGraph Graph;
  PUNGraph Graph1;
  PUNGraph Graph2;
  int NCount, ECount;
  int x,y;
  TIntV NodeIds;
  THashSet<TIntPr> EdgeSet;
  TInt::Rnd.PutSeed(0);
  
  for (int i = 0; i < Iterations; i++) {
    
    for (NEdges = NEdgesStart; NEdges <= NEdgesEnd; NEdges++) {
      
      for (NNodes = NNodesStart; NNodes <= NNodesEnd; NNodes++) {
        
        // Skip if too many edges required per NNodes (n*(n+1)/2)
        if (NEdges > (NNodes * (NNodes+1)/2)) {
          continue;
        }
        
        Graph = TUNGraph::New();
        EXPECT_TRUE(Graph->Empty());
        
        // Generate NNodes
        NodeIds.Gen(NNodes);
        for (int n = 0; n < NNodes; n++) {
          NodeIds[n] = n;
        }
        // Add the nodes in random order
        NodeIds.Shuffle(TInt::Rnd);
        for (int n = 0; n < NodeIds.Len(); n++) {
          Graph->AddNode(NodeIds[n]);
        }
        EXPECT_FALSE(Graph->Empty());
        
        // Create random edges
        EdgeSet.Clr();
        NCount = NEdges;
        while (NCount > 0) {
          x = (long) (drand48() * NNodes);
          y = (long) (drand48() * NNodes);
          
          if (!Graph->IsEdge(x,y)) {
            Graph->AddEdge(x, y);
            EdgeSet.AddKey(TIntPr(x, y));
            EdgeSet.AddKey(TIntPr(y, x));
            NCount--;
          }
        }
        
        // Check edge iterator to make sure all edges are valid and no more (in hash set)
        TIntPrV DelEdgeV;
        for (TUNGraph::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
          
          TIntPr Edge(EI.GetSrcNId(), EI.GetDstNId());
          TIntPr EdgeR(EI.GetDstNId(), EI.GetSrcNId());
          
          EXPECT_TRUE(EdgeSet.IsKey(Edge) || EdgeSet.IsKey(EdgeR));
          if (EdgeSet.IsKey(Edge)) {
              EdgeSet.DelKey(Edge);
          }
          if (EdgeSet.IsKey(EdgeR)) {
            EdgeSet.DelKey(EdgeR);
          }
          DelEdgeV.Add(Edge);
          
        }
        EXPECT_TRUE(EdgeSet.Empty());
        EXPECT_TRUE(DelEdgeV.Len() == NEdges);
        
        // Randomly delete node, check to make sure edges were deleted
        NodeIds.Shuffle(TInt::Rnd);
        for (int n = 0; n < NNodes; n++) {

          TIntPrV DelEdgeNodeV;
          int DelNodeId = NodeIds[n];
          
          // Get edges for node to be deleted (to verify all deleted)
          int EdgesBeforeDel;
          EdgesBeforeDel = Graph->GetEdges();
          for (TUNGraph::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
            if (EI.GetSrcNId() == DelNodeId || EI.GetDstNId() == DelNodeId) {
              DelEdgeNodeV.Add(TIntPr(EI.GetSrcNId(), EI.GetDstNId()));
            }
          }
          
          Graph->DelNode(DelNodeId);
          EXPECT_TRUE(EdgesBeforeDel == DelEdgeNodeV.Len() + Graph->GetEdges());
          EXPECT_FALSE(Graph->IsNode(DelNodeId));
          EXPECT_TRUE(Graph->IsOk());
          
          // Make sure all the edges to deleted node are gone
          for (int e = 0; e < DelEdgeNodeV.Len(); e++) {
            EXPECT_FALSE(Graph->IsEdge(DelEdgeNodeV[e].Val1, DelEdgeNodeV[e].Val2));
          }
          
          // Make sure no edge on graph is connected to deleted node
          ECount = 0;
          for (TUNGraph::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
            EXPECT_FALSE(EI.GetSrcNId() == DelNodeId || EI.GetDstNId() == DelNodeId);
            ECount++;
          }
          // Make sure iterator returns all of the edges
          EXPECT_TRUE(ECount == Graph->GetEdges());
        }

        EXPECT_TRUE(0 == Graph->GetEdges());
        Graph->Clr();
        EXPECT_TRUE(Graph->Empty());
      }
    }
  }

}

// Test small graph
TEST(TUNGraph, GetSmallGraph) {
  PUNGraph Graph;

  Graph = TUNGraph::GetSmallGraph();

  EXPECT_EQ(5,Graph->GetNodes());
  EXPECT_EQ(5,Graph->GetEdges());

  EXPECT_EQ(1,Graph->IsOk());
  EXPECT_EQ(0,Graph->Empty());
  EXPECT_EQ(0,Graph->HasFlag(gfDirected));
}

// Test node, edge creation
TEST(TUNGraph, ManipulateNodesEdgesWith64BitNodeValues) {
  int64 NNodes = 10000;
  int64 NEdges = 100000;
  int64 OFFSET = 1000000000000;
  const char *FName = "test.graph.dat";

  PUNGraph Graph;
  PUNGraph Graph1;
  PUNGraph Graph2;
  int64 i;
  int64 n;
  int64 NCount;
  int64 LCount;
  int64 x,y;
  int64 Deg, InDeg, OutDeg;

  Graph = TUNGraph::New();
  EXPECT_EQ(1,Graph->Empty());

  // create the nodes
  for (i = 0; i < NNodes; i++) {
    Graph->AddNode(OFFSET + i);
  }
  EXPECT_EQ(0,Graph->Empty());
  EXPECT_EQ(NNodes,Graph->GetNodes());

  // create random edges
  NCount = NEdges;
  LCount = 0;
  while (NCount > 0) {
    x = (long) (drand48() * NNodes);
    y = (long) (drand48() * NNodes);
    if (!Graph->IsEdge(OFFSET + x,OFFSET + y)) {
      n = Graph->AddEdge(OFFSET +x,OFFSET + y);
      NCount--;
      if (x == y) {
        LCount++;
      }
    }
  }

  EXPECT_EQ(NEdges,Graph->GetEdges());

  EXPECT_EQ(0,Graph->Empty());
  EXPECT_EQ(1,Graph->IsOk());

  for (i = 0; i < NNodes; i++) {
    EXPECT_EQ(1,Graph->IsNode(OFFSET + i));
  }

  EXPECT_EQ(0,Graph->IsNode(OFFSET + NNodes));
  EXPECT_EQ(0,Graph->IsNode(OFFSET + NNodes+1));
  EXPECT_EQ(0,Graph->IsNode(OFFSET + 2*NNodes));

  // nodes iterator
  NCount = 0;
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    NCount++;
  }
  EXPECT_EQ(NNodes,NCount);

  // edges per node iterator, each non-loop is visited twice, each loop once
  NCount = 0;
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    for (int e = 0; e < NI.GetOutDeg(); e++) {
      NCount++;
    }
  }
  EXPECT_EQ(NEdges*2-LCount,NCount);

  // edges iterator, each edge is visited once
  NCount = 0;
  for (TUNGraph::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
    NCount++;
  }
  EXPECT_EQ(NEdges,NCount);

  // node degree
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    Deg = NI.GetDeg();
    InDeg = NI.GetInDeg();
    OutDeg = NI.GetOutDeg();

    EXPECT_EQ(Deg,InDeg);
    EXPECT_EQ(Deg,OutDeg);
  }

  // assignment
  Graph1 = TUNGraph::New();
  *Graph1 = *Graph;

  EXPECT_EQ(NNodes,Graph1->GetNodes());
  EXPECT_EQ(NEdges,Graph1->GetEdges());
  EXPECT_EQ(0,Graph1->Empty());
  EXPECT_EQ(1,Graph1->IsOk());

  // saving and loading
  {
    TFOut FOut(FName);
    Graph->Save(FOut);
    FOut.Flush();
  }

  {
    TFIn FIn(FName);
    Graph2 = TUNGraph::Load(FIn);
  }

  EXPECT_EQ(NNodes,Graph2->GetNodes());
  EXPECT_EQ(NEdges,Graph2->GetEdges());
  EXPECT_EQ(0,Graph2->Empty());
  EXPECT_EQ(1,Graph2->IsOk());

  // remove all the nodes and edges
  for (i = 0; i < NNodes; i++) {
    n = Graph->GetRndNId();
    Graph->DelNode(n);
  }

  EXPECT_EQ(0,Graph->GetNodes());
  EXPECT_EQ(0,Graph->GetEdges());

  EXPECT_EQ(1,Graph->IsOk());
  EXPECT_EQ(1,Graph->Empty());

  Graph1->Clr();

  EXPECT_EQ(0,Graph1->GetNodes());
  EXPECT_EQ(0,Graph1->GetEdges());

  EXPECT_EQ(1,Graph1->IsOk());
  EXPECT_EQ(1,Graph1->Empty());
}

// Test edge iterator, manipulate edges
TEST(TUNGraph, ManipulateEdgesWith64BitNodeValues) {
  int64 Iterations = 100;

  int64 NNodes;
  int64 NNodesStart = 8;
  int64 NNodesEnd = 25;

  int64 NEdges;
  int64 NEdgesStart = 0;
  int64 NEdgesEnd = 50;

  PUNGraph Graph;
  PUNGraph Graph1;
  PUNGraph Graph2;
  int64 NCount, ECount;
  int64 x,y;
  TInt64V NodeIds;
  int64 OFFSET = 1000000000000;
  THashSet<TInt64Pr, int64> EdgeSet;
  TInt::Rnd.PutSeed(0);

  for (int i = 0; i < Iterations; i++) {

    for (NEdges = NEdgesStart; NEdges <= NEdgesEnd; NEdges++) {

      for (NNodes = NNodesStart; NNodes <= NNodesEnd; NNodes++) {

        // Skip if too many edges required per NNodes (n*(n+1)/2)
        if (NEdges > (NNodes * (NNodes+1)/2)) {
          continue;
        }

        Graph = TUNGraph::New();
        EXPECT_TRUE(Graph->Empty());

        // Generate NNodes
        NodeIds.Gen(NNodes);
        for (int n = 0; n < NNodes; n++) {
          NodeIds[n] = OFFSET + n;
        }
        // Add the nodes in random order
        NodeIds.Shuffle(TInt::Rnd);
        for (int n = 0; n < NodeIds.Len(); n++) {
          Graph->AddNode(NodeIds[n]);
        }
        EXPECT_FALSE(Graph->Empty());

        // Create random edges
        EdgeSet.Clr();
        NCount = NEdges;
        while (NCount > 0) {
          x = (long) (drand48() * NNodes);
          y = (long) (drand48() * NNodes);

          if (!Graph->IsEdge(OFFSET + x,OFFSET + y)) {
            Graph->AddEdge(OFFSET + x, OFFSET + y);
            EdgeSet.AddKey(TInt64Pr(OFFSET + x, OFFSET + y));
            EdgeSet.AddKey(TInt64Pr(OFFSET + y, OFFSET + x));
            NCount--;
          }
        }

        // Check edge iterator to make sure all edges are valid and no more (in hash set)
        TIntPr64V DelEdgeV;
        for (TUNGraph::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {

          TInt64Pr Edge(EI.GetSrcNId(), EI.GetDstNId());
          TInt64Pr EdgeR(EI.GetDstNId(), EI.GetSrcNId());

          EXPECT_TRUE(EdgeSet.IsKey(Edge) || EdgeSet.IsKey(EdgeR));
          if (EdgeSet.IsKey(Edge)) {
              EdgeSet.DelKey(Edge);
          }
          if (EdgeSet.IsKey(EdgeR)) {
            EdgeSet.DelKey(EdgeR);
          }
          DelEdgeV.Add(Edge);

        }
        EXPECT_TRUE(EdgeSet.Empty());
        EXPECT_TRUE(DelEdgeV.Len() == NEdges);

        // Randomly delete node, check to make sure edges were deleted
        NodeIds.Shuffle(TInt::Rnd);
        for (int n = 0; n < NNodes; n++) {

          TIntPr64V DelEdgeNodeV;
          int64 DelNodeId = NodeIds[n];

          // Get edges for node to be deleted (to verify all deleted)
          int64 EdgesBeforeDel;
          EdgesBeforeDel = Graph->GetEdges();
          for (TUNGraph::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
            if (EI.GetSrcNId() == DelNodeId || EI.GetDstNId() == DelNodeId) {
              DelEdgeNodeV.Add(TInt64Pr(EI.GetSrcNId(), EI.GetDstNId()));
            }
          }

          Graph->DelNode(DelNodeId);
          EXPECT_TRUE(EdgesBeforeDel == DelEdgeNodeV.Len() + Graph->GetEdges());
          EXPECT_FALSE(Graph->IsNode(DelNodeId));
          EXPECT_TRUE(Graph->IsOk());

          // Make sure all the edges to deleted node are gone
          for (int e = 0; e < DelEdgeNodeV.Len(); e++) {
            EXPECT_FALSE(Graph->IsEdge(DelEdgeNodeV[e].Val1, DelEdgeNodeV[e].Val2));
          }

          // Make sure no edge on graph is connected to deleted node
          ECount = 0;
          for (TUNGraph::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
            EXPECT_FALSE(EI.GetSrcNId() == DelNodeId || EI.GetDstNId() == DelNodeId);
            ECount++;
          }
          // Make sure iterator returns all of the edges
          EXPECT_TRUE(ECount == Graph->GetEdges());
        }

        EXPECT_TRUE(0 == Graph->GetEdges());
        Graph->Clr();
        EXPECT_TRUE(Graph->Empty());
      }
    }
  }

}

// Test node, edge creation
TEST(TUNGraph, ManipulateNodesEdges64Bit) {
  int64 NNodes = 100000000;
  int64 NOutDeg = 50; // should be < NNodes
  int64 NEdges = NOutDeg*NNodes + NNodes;
  const int64 OFFSET = 0;
  const char *FName = "test.graph.dat";

  PUNGraph Graph;
  PUNGraph Graph1;
  PUNGraph Graph2;
  int64 i,j;
  int64 n;
  int64 LCount = 0, NCount = 0;
  int64 Deg, InDeg, OutDeg;
  Graph = TUNGraph::New();
  EXPECT_EQ(1,Graph->Empty());
  std::cerr<<"Creating nodes\n";
  // create the nodes
  for (i = 0; i < NNodes; i++) {
    Graph->AddNode(OFFSET + i);
  }
  EXPECT_EQ(0,Graph->Empty());
  EXPECT_EQ(NNodes,Graph->GetNodes());
  std::cerr<<"Creating random edges\n";

  // create random edges
  for (i=0; i < NNodes; i++){
  	for (j = 1; j <= NOutDeg; j++){
  		Graph->AddEdge(OFFSET +i,OFFSET + (i + j)%NNodes);
  		NCount++;
  	}
		Graph->AddEdge(OFFSET + i,OFFSET + i);
		NCount++;
		LCount++;
		if (i%10000000 == 0)
			std::cerr<<i/10000000<<std::endl;
  }

  EXPECT_EQ(NEdges,NCount);
  EXPECT_EQ(NEdges,Graph->GetEdges());

  EXPECT_EQ(0,Graph->Empty());
  EXPECT_EQ(1,Graph->IsOk());

  for (i = 0; i < NNodes; i++) {
    EXPECT_EQ(1,Graph->IsNode(OFFSET + i));
  }

  EXPECT_EQ(0,Graph->IsNode(OFFSET + NNodes));
  EXPECT_EQ(0,Graph->IsNode(OFFSET + NNodes+1));
  EXPECT_EQ(0,Graph->IsNode(OFFSET + 2*NNodes));
  std::cerr<<"nodes iterator\n";

  // nodes iterator
  NCount = 0;
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    NCount++;
  }
  EXPECT_EQ(NNodes,NCount);
  std::cerr<<"edges per node iterator\n";

  // edges per node iterator, each non-loop is visited twice, each loop once
  NCount = 0;
  bool flag = false;
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    int64 nid = NI.GetId();
  	for (int64 e = 0; e < NI.GetOutDeg(); e++) {
    	int64 nbrid = NI.GetOutNId(e);
    	if (nid > OFFSET + 2*NOutDeg && nid < OFFSET + NNodes - 2*NOutDeg)
    		if (nbrid < nid - NOutDeg || nbrid > nid + NOutDeg)
    			flag = true;
      NCount++;
    }
  }
  EXPECT_EQ(NEdges*2-LCount,NCount);
  EXPECT_EQ(flag,false);
  std::cerr<<"Edge iterator\n";

  // edges iterator, each edge is visited once
  NCount = 0;
  for (TUNGraph::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
  	NCount++;
  }
  EXPECT_EQ(NEdges,NCount);
  std::cerr<<"Node degree\n";

  // node degree
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    Deg = NI.GetDeg();
    InDeg = NI.GetInDeg();
    OutDeg = NI.GetOutDeg();

    EXPECT_EQ(Deg,InDeg);
    EXPECT_EQ(Deg,OutDeg);
  }
  std::cerr<<"Assignment\n";

  // assignment
  Graph1 = TUNGraph::New();
  *Graph1 = *Graph;

  EXPECT_EQ(NNodes,Graph1->GetNodes());
  EXPECT_EQ(NEdges,Graph1->GetEdges());
  EXPECT_EQ(0,Graph1->Empty());
  EXPECT_EQ(1,Graph1->IsOk());
  std::cerr<<"Saving and loading\n";

  // saving and loading
  {
    TFOut FOut(FName);
    Graph->Save(FOut);
    FOut.Flush();
  }

  {
    TFIn FIn(FName);
    Graph2 = TUNGraph::Load(FIn);
  }

  EXPECT_EQ(NNodes,Graph2->GetNodes());
  EXPECT_EQ(NEdges,Graph2->GetEdges());
  EXPECT_EQ(0,Graph2->Empty());
  EXPECT_EQ(1,Graph2->IsOk());
  std::cerr<<"remove all nodes and edges\n";

  // remove all the nodes and edges
  for (i = 0; i < NNodes; i++) {
    n = Graph->GetRndNId();
    Graph->DelNode(n);
    if(i%1000000 == 0)
        	std::cerr<<i/1000000<<std::endl;
  }

  EXPECT_EQ(0,Graph->GetNodes());
  EXPECT_EQ(0,Graph->GetEdges());

  EXPECT_EQ(1,Graph->IsOk());
  EXPECT_EQ(1,Graph->Empty());
  std::cerr<<"Clear graph\n";

  Graph1->Clr();

  EXPECT_EQ(0,Graph1->GetNodes());
  EXPECT_EQ(0,Graph1->GetEdges());

  EXPECT_EQ(1,Graph1->IsOk());
  EXPECT_EQ(1,Graph1->Empty());
}


