//***************************************************************************
// Copyright 2007-2013 Universidade do Porto - Faculdade de Engenharia      *
// Laboratório de Sistemas e Tecnologia Subaquática (LSTS)                  *
//***************************************************************************
// This file is part of DUNE: Unified Navigation Environment.               *
//                                                                          *
// Commercial Licence Usage                                                 *
// Licencees holding valid commercial DUNE licences may use this file in    *
// accordance with the commercial licence agreement provided with the       *
// Software or, alternatively, in accordance with the terms contained in a  *
// written agreement between you and Universidade do Porto. For licensing   *
// terms, conditions, and further information contact lsts@fe.up.pt.        *
//                                                                          *
// European Union Public Licence - EUPL v.1.1 Usage                         *
// Alternatively, this file may be used under the terms of the EUPL,        *
// Version 1.1 only (the "Licence"), appearing in the file LICENCE.md       *
// included in the packaging of this file. You may not use this work        *
// except in compliance with the Licence. Unless required by applicable     *
// law or agreed to in writing, software distributed under the Licence is   *
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF     *
// ANY KIND, either express or implied. See the Licence for the specific    *
// language governing permissions and limitations at                        *
// https://www.lsts.pt/dune/licence.                                        *
//***************************************************************************
// Author: Joao Fortuna                                                     *
//***************************************************************************

// DUNE headers.
#include <DUNE/DUNE.hpp>

// Local headers.
#include "Client.hpp"

namespace Transports
{
  namespace SerialOverTCP
  {
    using DUNE_NAMESPACES;

    Client::Client(TCPSocket* sock):
      m_sock(sock),
      m_timer(5)
    { }

    Client::~Client(void)
    {
      closeConnection();
    }

    int
    Client::read(char* bfr)
    {
      if(m_out_data.empty())
        return 0;
      else
      {
        bfr = m_out_data.front();
        m_out_data.pop();
        return sizeof(bfr);
      }
    }

    void
    Client::write(char* bfr)
    {
      m_in_data.push(bfr);
    }

    void
    Client::closeConnection(void)
    {
      if (m_sock == NULL)
        return;

      m_poll.remove(*m_sock);
      delete m_sock;
      m_sock = NULL;
    }

    void
    Client::run(void)
    {
      m_poll.add(*m_sock);

      m_timer.reset();

      while (!isStopping())
      {
        if (m_timer.overflow())
          break;

        try
        {
          if (!m_poll.poll(1.0))
            continue;

          if (!m_poll.wasTriggered(*m_sock))
            continue;

          int rv = m_sock->read(m_bfr, sizeof(m_bfr));
          if (rv <= 0)
            break;

          m_out_data.push(m_bfr);
          m_timer.reset();
        }
        catch (...)
        {
          break;
        }

        try
        {
          if(!m_in_data.empty())
          {
            m_sock->write(m_in_data.front(), sizeof(m_in_data.front()));
            m_in_data.pop();
          }
        }
        catch (...)
        {
          continue;
        }

      }

      closeConnection();
    }
  }
}